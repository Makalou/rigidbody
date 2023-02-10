#include "Application.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include <queue>
#include "glm/gtc/quaternion.hpp"

float lastFrame = 0.0f;

void Application::init_window() {
    window_system = std::make_unique<subsystem::WindowSystem>();
    window_system->init();
}

void Application::init_GPU_backend(){
    gpu_backend = std::make_unique<MyBackend>();
    gpu_backend->init(window_system->get_window());
}
glm::vec3 P = {};
glm::vec3 L = {0,0,0};

std::vector<float> v_y;

bool physics_sim_paused = true;
bool collide = false;
bool rota = false;

int collide_count = 0;

void Application::mainloop() {
    while (!window_system->window_should_close()) {
        glfwPollEvents();
        auto currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if(!physics_sim_paused) {
            physics_system->update(deltaTime);
            gpu_backend->mesh->upload_vertex_data_to_device(VulkanDevice{gpu_backend->vkb_device});
        }
        updateGUI();
        drawFrame();
    }
    vkDeviceWaitIdle(gpu_backend->vkb_device);
}

void Application::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    gpu_backend->cleanup();
    window_system->cleanup();
    glfwTerminate();
}

size_t currentFrame = 0;

void Application::drawFrame() {

    vkWaitForFences(gpu_backend->vkb_device, 1, &gpu_backend->inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    uint32_t imageIndex;
    auto aquiresult = vkAcquireNextImageKHR(gpu_backend->vkb_device, gpu_backend->get_swapChain(),
                                            std::numeric_limits<uint64_t>::max(),
                                            gpu_backend->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                                            &imageIndex);

    if (aquiresult == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        gpu_backend->recreateSwapChain();
        return;
    } else if (aquiresult != VK_SUCCESS && aquiresult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    auto aspect = gpu_backend->vkb_swapchain.extent.width / (float) gpu_backend->vkb_swapchain.extent.height;
    gpu_backend->resourceManager.updateUniformBuffer(imageIndex, rota);
    gpu_backend->resourceManager.updateCameraBuffer(imageIndex);//todo should move to other place
    //gpu_backend->resourceManager.updateModelMatrix(imageIndex,aspect,x,q);
    VkSemaphore waitSemaphores[] = {gpu_backend->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    auto command = gpu_backend->commandBuffers[imageIndex];
    auto gui_command = gpu_backend->gui_commandBuffers[imageIndex];

    renderGUI(gui_command, imageIndex);

    VkSemaphore signalSemaphores[] = {gpu_backend->renderFinishedSemaphores[currentFrame]};

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
    vkResetFences(gpu_backend->vkb_device, 1, &gpu_backend->inFlightFences[currentFrame]);
    if (vkQueueSubmit(gpu_backend->vkb_device.get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo,
                      gpu_backend->inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {gpu_backend->get_swapChain()};
    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex
    };

    auto qp_result = vkQueuePresentKHR(gpu_backend->vkb_device.get_queue(vkb::QueueType::present).value(), &presentInfo);
    if (qp_result == VK_ERROR_OUT_OF_DATE_KHR || qp_result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        gpu_backend->recreateSwapChain();
    } else if (qp_result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

void Application::init_gui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window_system->get_window(), true);
    ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = gpu_backend->vkb_inst,//gpu_backend->vk_instance_wrapper.get_vk_instance(),
            .PhysicalDevice = gpu_backend->vkb_device.physical_device,
            .Device = gpu_backend->getDevice(),
            //todo .quefamily
            .Queue = gpu_backend->vkb_device.get_queue(vkb::QueueType::graphics).value(),
            .PipelineCache = VK_NULL_HANDLE,
            .DescriptorPool = gpu_backend->imguiDescriptorPool.m_pool,
            .MinImageCount = static_cast<uint32_t>(gpu_backend->vkb_swapchain.image_count),
            .ImageCount = static_cast<uint32_t>(gpu_backend->vkb_swapchain.image_count),
            .Allocator = VK_NULL_HANDLE,
            .CheckVkResultFn = nullptr
    };
    ImGui_ImplVulkan_Init(&init_info, gpu_backend->imGuiPass);

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

        ImGui::Checkbox("Simulation Paused",&physics_sim_paused);

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

void Application::init_physics_system() {
    physics_system = std::make_unique<subsystem::PhysicsSystem>();
    physics_system->init(gpu_backend->mesh);
    gpu_backend->mesh->upload_vertex_data_to_device(VulkanDevice{gpu_backend->vkb_device});
}

void Application::renderGUI(VkCommandBuffer commandBuffer, uint32_t index) {
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = gpu_backend->imGuiPass,
            .framebuffer = gpu_backend->imGuiPass.m_framebuffers[index].m_frame_buffer,
            .renderArea.extent.width = gpu_backend->vkb_swapchain.extent.width,
            .renderArea.extent.height = gpu_backend->vkb_swapchain.extent.height
    };

    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}

void Application::init_scene() {
    scene = std::make_unique<scene::SceneGraph>();

    auto floor = scene->addNodeToRoot("floor");
    floor->addComponent(std::make_unique<scene::MeshRenderer>());
    auto main_light = scene->addNodeToRoot("mainLight");
    main_light->addComponent(std::make_unique<scene::Light>());
    auto cam = scene->addNodeToRoot("mainCamera");
    cam->addComponent(std::make_unique<scene::Viewer>());

    auto R2D2 = floor->addChild("R2-D2");
    auto dragon0 = floor->addChild("dragon0");
    auto dragon1 = floor->addChild("dragon1");
    auto dragon2 = floor->addChild("dragon2");
    auto dragon3 = floor->addChild("dragon3");
    auto bunny = floor->addChild("bunny");

    R2D2->addComponent(std::make_unique<scene::MeshRenderer>());
    dragon0->addComponent(std::make_unique<scene::MeshRenderer>());
    dragon1->addComponent(std::make_unique<scene::MeshRenderer>());
    dragon2->addComponent(std::make_unique<scene::MeshRenderer>());
    dragon3->addComponent(std::make_unique<scene::MeshRenderer>());
    bunny->addComponent(std::make_unique<scene::MeshRenderer>());
}

void Application::init_renderer() {
    render_system = std::make_unique<subsystem::RenderSystem>();
    render_system->init();
}

