#include "Application.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include <queue>

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
            gpu_backend->mesh->upload_vertex_data_to_device(VulkanDevice{gpu_backend->main_device});
        }
        updateGUI();
        drawFrame();
    }
    vkDeviceWaitIdle(gpu_backend->main_device);
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
    render_system->drawFrame(currentFrame,&frameBufferResized,rota);
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
            .PhysicalDevice = gpu_backend->main_device.physical_device,
            .Device = gpu_backend->getDevice(),
            //todo .quefamily
            .Queue = gpu_backend->main_device.get_queue(vkb::QueueType::graphics).value(),
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
    gpu_backend->mesh->upload_vertex_data_to_device(VulkanDevice{gpu_backend->main_device});
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
    render_system->init(gpu_backend.get());
}

