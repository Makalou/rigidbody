#include "Application.h"
#include "vk_context.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include <queue>
#include "glm/gtc/quaternion.hpp"
#include "simd/simd.h"

static void frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->setFrameBufferResized(true);
    std::cout<<"window resize: ["<<width<<","<<height<<"]\n";
}

static Camera *pCamera = new Camera();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static void cursorPosCallback(GLFWwindow *window, double x, double y) {
    static double old_x = -1;
    static double old_y = -1;
    if (old_x == -1 || old_y == -1) {
        old_x = x;
        old_y = y;
        return;
    }
    auto x_offset = x - old_x;
    old_x = x;
    auto y_offset = old_y - y;
    old_y = y;
    pCamera->deflect(x_offset, y_offset);
}

auto currentMouseVisible = GLFW_CURSOR_NORMAL;


static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_W:
            pCamera->transfrom(CameraMovement::FORWARD, deltaTime);
            break;
        case GLFW_KEY_S:
            pCamera->transfrom(CameraMovement::BACKWARD, deltaTime);
            break;
        case GLFW_KEY_A:
            pCamera->transfrom(CameraMovement::LEFT, deltaTime);
            break;
        case GLFW_KEY_D:
            pCamera->transfrom(CameraMovement::RIGHT, deltaTime);
            break;
        case GLFW_KEY_H:
            if(action == GLFW_PRESS){
                if (currentMouseVisible == GLFW_CURSOR_NORMAL) {
                    currentMouseVisible = GLFW_CURSOR_DISABLED;
                    glfwSetCursorPosCallback(window, cursorPosCallback);
                } else {
                    currentMouseVisible = GLFW_CURSOR_NORMAL;
                    glfwSetCursorPosCallback(window, nullptr);
                }
                glfwSetInputMode(window, GLFW_CURSOR, currentMouseVisible);
            }
            break;

        default:
            break;
    }
}

static void scrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
    pCamera->zoom(y_offset);
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

    }
}

void Application::init_window() {
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("glfwinit failed!");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIN_WIDTH, WIN_HIGHT, WIN_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    Camera::registry("camera1", pCamera);
}

void Application::init_vulkan() {
    my_context = new MyContext;
    my_context->init(window);
}

const auto scale = 1.0f;

glm::vec3 x = {0.f,5.f,0.f};
glm::qua<float> q{glm::radians(glm::vec3 (0.f,0.f,0.f))};
glm::vec3 P = {};
glm::vec3 L = {0,0,0};

const glm::vec3 g = {0.f, -9.8f, 0.f};
const float damping = 0.994f;

std::vector<float> v_y;
std::vector<float> energy;

size_t vex_num;

bool paused = true;
bool collide = false;
bool rota = false;

int collide_count = 0;

bool right_side(const glm::vec3& plane_point,const glm::vec3& plane_normal,const glm::vec3& point){
    return glm::dot(plane_normal,point-plane_point)<=0;
}

std::vector<glm::vec3> convex_hull(const std::vector<glm::vec3> points){
    std::vector<glm::vec3> result;
    for(int i=0;i<points.size();++i){
        for(int j =i+1;j<points.size();++j){
            for(int k = j+1;k<points.size();++k){
                bool valid = true;
                auto plane_normal = glm::cross(points[j]-points[i],points[k]-points[i]);
                for(int p=0;p<points.size();p++){
                    if(p==i||p==j||p==k)
                        continue;
                    valid&=right_side(plane_normal,points[i],points[p]);
                    if(!valid) break;
                }
                if(valid) {
                    result.push_back(points[i]);
                    result.push_back(points[j]);
                    result.push_back(points[k]);
                }
            }
        }
    }
    return result;
}

void Application::mainloop() {

    auto vertex_positions = my_context->mesh->get_position_view();

    std::vector<glm::vec3> local_state(vertex_positions.size());

    const float mass = 10.0f;
    const float per_vert_mass = mass/local_state.size();
    const auto gravity = mass*g;

    for(size_t i =0;i<local_state.size();++i)
        local_state[i] = *vertex_positions[i];

    glm::vec3 center{};
    for (auto & p: local_state) {
        center+=p;
    }

    center/=local_state.size();

    float collision_radius = 0;
    for (auto & pos: local_state) {
        pos-=center;
        collision_radius = fmax(collision_radius,glm::length(pos));
        //pos*=0.2;
    }

    //convex_hull(local_state);

    float w11 = 0,w12=0,w13=0,w22=0,w23=0,w33=0;

    for (auto & pos: local_state) {
        w11 += (pos.y*pos.y+pos.z*pos.z)*per_vert_mass;
        w12 -= pos.x*pos.y*per_vert_mass;
        w13 -= pos.x*pos.z*per_vert_mass;
        w22 += (pos.x*pos.x+pos.z*pos.z)*per_vert_mass;
        w23 -= pos.y*pos.z*per_vert_mass;
        w33 += (pos.x*pos.x+pos.y*pos.y)*per_vert_mass;
    }

    const glm::mat3 I0_inv = glm::inverse(glm::mat3{w11,w12,w13,w12,w22,w23,w13,w23,w33});

    auto R = glm::mat3_cast(q);
    for(size_t i =0;i<vertex_positions.size();++i){
        vertex_positions[i]=x+R*local_state[i];
    }

    my_context->mesh->upload_vertex_data_to_device(VulkanDevice{my_context->vkb_device});

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if(!paused) {
                auto dx = P / mass;
                R = glm::mat3_cast(q);
                auto I_inv = R * I0_inv * glm::transpose(R);
                glm::vec3 w = I_inv * L;
                //glm::mat3 w_star = {0,w.z,-w.y,-w.z,0,w.x,w.y,-w.x,0};
                //std::cout<<"["<<w.x<<","<<w.y<<","<<w.z<<"]";
                //std::cout<<glm::determinant(glm::mat3_cast(q))<<"\n";
                auto wq = glm::qua(0.f,w);
                auto dq = 0.5f*wq*q;
                //std::cout<<"dq : "<<"["<<dq.w<<","<<dq.x<<","<<dq.y<<","<<dq.z<<"]\n";
                //std::cout<<"q : "<<"["<<q.w<<","<<q.x<<","<<q.y<<","<<q.z<<"]\n";
                x += dx*deltaTime;
                q += dq*deltaTime;
                q = glm::normalize(q);
                R = glm::mat3_cast(q);
                P += gravity * deltaTime;
                P *= damping;L *= damping;
                for (size_t i = 0; i < vertex_positions.size(); ++i) {
                    vertex_positions[i] = x + R * local_state[i];
                }
#define ENABLE_COLLISION
#ifdef ENABLE_COLLISION
if(center.y-collision_radius<=0){
                glm::vec3 deltaP{};
                glm::vec3 deltaL{};
                collide_count = 0;
                for (size_t i = 0; i < vertex_positions.size(); ++i) {
                    auto p = vertex_positions[i];
                    if (p->y < 0.00f&&p->x<=2.0f&&p->x>=-2.0f&&p->z<=2.0f&&p->z>=-2.0f) {
                        auto r = R*local_state[i];
                        float v_normal = (P / mass + glm::cross(w, r)).y;
                        if (v_normal<=0) {
                            //std::cout<<v_normal<<"\n";
                            float j1 = -(1.f + 0.3f) * v_normal;
                            float j2 = 1 / mass + (I_inv * glm::cross(glm::cross(r, {0.f, 1.0f, 0.f}), r)).y;
                            float j = j1 / j2;
                            auto J = glm::vec3{0, j, 0};
                            deltaP += J;
                            deltaL += glm::cross(r, J);
                            collide_count++;
                        }
                    }
                }
                if(collide_count>0) {
                    deltaP/=collide_count;deltaL/=collide_count;
                    //std::cout<<collide_count<<"\n";
                }
                P += deltaP;L += deltaL;

}
#endif
            my_context->mesh->upload_vertex_data_to_device(VulkanDevice{my_context->vkb_device});
        }
        updateGUI();
        drawFrame();
    }
    vkDeviceWaitIdle(my_context->vkb_device);
}

void Application::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    my_context->cleanup();
    delete my_context;
    glfwDestroyWindow(window);
    glfwTerminate();
}

size_t currentFrame = 0;

void Application::drawFrame() {

    vkWaitForFences(my_context->vkb_device, 1, &my_context->inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    uint32_t imageIndex;
    auto aquiresult = vkAcquireNextImageKHR(my_context->vkb_device, my_context->get_swapChain(),
                                            std::numeric_limits<uint64_t>::max(),
                                            my_context->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                                            &imageIndex);

    if (aquiresult == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        my_context->recreateSwapChain();
        return;
    } else if (aquiresult != VK_SUCCESS && aquiresult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    auto aspect = my_context->vkb_swapchain.extent.width /(float) my_context->vkb_swapchain.extent.height;
    my_context->resourceManager.updateUniformBuffer(imageIndex,rota);
    my_context->resourceManager.updateCameraBuffer(imageIndex);
    //my_context->resourceManager.updateModelMatrix(imageIndex,aspect,x,q);
    VkSemaphore waitSemaphores[] = {my_context->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    auto command = my_context->commandBuffers[imageIndex];
    auto gui_command = my_context->gui_commandBuffers[imageIndex];

    renderGUI(gui_command, imageIndex);

    VkSemaphore signalSemaphores[] = {my_context->renderFinishedSemaphores[currentFrame]};

    VkCommandBuffer commandBuffers[] = {command, gui_command};

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = std::size(commandBuffers),
            .pCommandBuffers = commandBuffers,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
    };

    //render queue -> producer
    //present queue -> consumer
    //swapchain -> mailbox
    vkResetFences(my_context->vkb_device, 1, &my_context->inFlightFences[currentFrame]);
    if (vkQueueSubmit(my_context->vkb_device.get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo,
                      my_context->inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {my_context->get_swapChain()};
    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex
    };

    auto qp_result = vkQueuePresentKHR(my_context->vkb_device.get_queue(vkb::QueueType::present).value(), &presentInfo);
    if (qp_result == VK_ERROR_OUT_OF_DATE_KHR || qp_result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        my_context->recreateSwapChain();
    } else if (qp_result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    //vkQueueWaitIdle(my_context->presentQueue);

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

void Application::init_gui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = my_context->vkb_inst,//my_context->vk_instance_wrapper.get_vk_instance(),
            .PhysicalDevice = my_context->vkb_device.physical_device,
            .Device = my_context->getDevice(),
            //todo .quefamily
            .Queue = my_context->vkb_device.get_queue(vkb::QueueType::graphics).value(),
            .PipelineCache = VK_NULL_HANDLE,
            .DescriptorPool = my_context->imguiDescriptorPool.m_pool,
            .MinImageCount = static_cast<uint32_t>(my_context->vkb_swapchain.image_count),
            .ImageCount = static_cast<uint32_t>(my_context->vkb_swapchain.image_count),
            .Allocator = VK_NULL_HANDLE,
            .CheckVkResultFn = nullptr
    };
    ImGui_ImplVulkan_Init(&init_info, my_context->imGuiRenderPass);

    auto command_buffer = VulkanCommandManager::instance().beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    VulkanCommandManager::instance().endandSubmitSingleTimeCommands(command_buffer);

}

void Application::updateGUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
    {
        ImGui::Begin("Graphs");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Checkbox("View rotation",&rota);

        ImGui::Checkbox("Simulation Paused",&paused);

        ImGui::Text("momentum: [%.3f,%.3f,%.3f]",P.x,P.y,P.z);
        ImGui::Text("angler momentum: [%.3f,%.3f,%.3f]",L.x,L.y,L.z);

        //if(ImGui::Button("New Cube"))

        ImGui::Text("Collide: %d",collide_count);
        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.

        /*
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = ImGui::GetTime();
        while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }
         */
        if (v_y.size()>10000)
            v_y.erase(v_y.begin());
        // Plots can display overlay texts
        // (in this example, we will display an average value)
        /*
        {
            float average = 0.0f;
            for (int n = 0; n < v_y.size(); n++)
                average += values[n];
            average /= (float)IM_ARRAYSIZE(values);
            char overlay[32];
            sprintf(overlay, "avg %f", average);
        }
         */

        ImGui::End();
    }

    ImGui::Render();
}

void Application::renderGUI(VkCommandBuffer commandBuffer, uint32_t index) {
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = my_context->imGuiRenderPass;
    info.framebuffer = my_context->imGuiRenderPass.m_framebuffers[index].m_frame_buffer;
    info.renderArea.extent.width = my_context->vkb_swapchain.extent.width;
    info.renderArea.extent.height = my_context->vkb_swapchain.extent.height;
    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}
