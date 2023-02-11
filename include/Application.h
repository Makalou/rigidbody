#pragma once
#define GLFW_INCLUDE_VULKAN
#include <stdexcept>
#include "vk_backend.h"
#include <memory>
#include "system/window_system.h"
#include "system/physics_system.h"
#include "system/render_system.h"
#include "scene_graph.h"
#include <thread>

class VulkanBackend;

class Application {
public:
	void run() {
        auto t1 = std::thread(
                [this]{init_scene();
                });
		init_window();
        init_GPU_backend();
        t1.join();
        init_renderer();
        init_physics_system();
        init_gui();
		mainloop();
		cleanup();
	}
	void setFrameBufferResized(bool flag) {
		frameBufferResized = flag;
	}

private:
    void init_scene();
	void init_window();
	void init_GPU_backend();
    void init_renderer();
    void init_gui();
    void init_physics_system();
	void mainloop();
	void cleanup();
    void updateGUI();
	void drawFrame();

	std::unique_ptr<scene::SceneGraph> scene;
    std::unique_ptr<subsystem::WindowSystem> window_system;
	std::unique_ptr<MyBackend> gpu_backend;
    std::unique_ptr<subsystem::PhysicsSystem> physics_system;
    std::unique_ptr<subsystem::RenderSystem> render_system;
	bool frameBufferResized=false;
};