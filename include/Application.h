#pragma once
#define GLFW_INCLUDE_VULKAN
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "win_config.h"
#include "vk_context.h"
#include <memory>

class VulkanBackend;

class Application {
public:
	void run() {
		init_window();
        init_graphics_backend();
        init_gui();
		mainloop();
		cleanup();
	}
	void setFrameBufferResized(bool flag) {
		frameBufferResized = flag;
	}
private:
	void init_window();
	void init_graphics_backend();
    void init_gui();
	void mainloop();
	void cleanup();
    void updateGUI();
    void renderGUI(VkCommandBuffer commandBuffer,uint32_t index);
	void drawFrame();
private:
	GLFWwindow* window;
	std::unique_ptr<MyBackend> my_backend;
	bool frameBufferResized=false;
};