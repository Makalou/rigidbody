#pragma once
#define GLFW_INCLUDE_VULKAN
#include <stdexcept>
#include "vk_backend.h"
#include <memory>
#include "system/window_system.h"
#include "system/physics_system.h"

class VulkanBackend;

class Application {
public:
	void run() {
		init_window();
        init_GPU_backend();
        init_physics_system();
        init_gui();
		mainloop();
		cleanup();
	}
	void setFrameBufferResized(bool flag) {
		frameBufferResized = flag;
	}
private:
	void init_window();
	void init_GPU_backend();
    void init_gui();
    void init_physics_system();
	void mainloop();
	void cleanup();
    void updateGUI();
    void renderGUI(VkCommandBuffer commandBuffer,uint32_t index);
	void drawFrame();
private:
	//GLFWwindow* window;
    std::unique_ptr<subsystem::WindowSystem> window_system;
	std::unique_ptr<MyBackend> gpu_backend;
    std::unique_ptr<subsystem::PhysicsSystem> physics_system;
	bool frameBufferResized=false;
};