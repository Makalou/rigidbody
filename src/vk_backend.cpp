#include "vk_backend.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "VkBootstrap.h"

void VulkanBackend::init(GLFWwindow *window) {
    this->window = window;
    vkb::InstanceBuilder instance_builder;
    auto inst_ret = instance_builder.set_app_name("phy sim")
            .request_validation_layers().use_default_debug_messenger().build();
    if (!inst_ret) std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
    vkb_inst = inst_ret.value();

    if (glfwCreateWindowSurface(vkb_inst, window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("failed to create window surface!");

    vkb::PhysicalDeviceSelector device_selector{vkb_inst};
    auto phys_ret = device_selector.set_surface(surface).set_minimum_version(1, 1).select();
    if (!phys_ret) std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto dev_ret = device_builder.build();
    if (!dev_ret) std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
    vkb_device = dev_ret.value();

    vkb::SwapchainBuilder swapchain_builder{vkb_device};
    auto swap_ret = swapchain_builder.set_old_swapchain(VK_NULL_HANDLE).set_desired_min_image_count(3).build();
    if (!swap_ret) std::cerr << "Failed to create Vulkan swapChain. Error: " << swap_ret.error().message() << "\n";
    vkb_swapchain = swap_ret.value();

    createSyncObjects();

    init_resource();
}

void VulkanBackend::cleanup() {
    for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        destroyDeviceObject(vkDestroySemaphore, renderFinishedSemaphores[i]);
        destroyDeviceObject(vkDestroySemaphore, imageAvailableSemaphores[i]);
        destroyDeviceObject(vkDestroyFence, inFlightFences[i]);
    }

    clean_resource();

    destroy_surface(vkb_inst,surface);
    destroy_device(vkb_device);
    destroy_instance(vkb_inst);
}

void VulkanBackend::createSyncObjects() {

    VkSemaphoreCreateInfo semaphoreInfo = {
            .sType =  VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vkb_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create image available semaphore!");
        }
        if(vkCreateSemaphore(vkb_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ){
            throw std::runtime_error("failed to create render finished semaphore!");
        }
        if(vkCreateFence(vkb_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create in flight fence!");
        }
    }
}


