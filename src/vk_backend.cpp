#include "vk_backend.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "VkBootstrap.h"

void VulkanBackend::init(GLFWwindow *window) {
    this->window = window;
    vkb::InstanceBuilder instance_builder;
    auto inst_ret = instance_builder.set_app_name("phy sim")
            .request_validation_layers()
            .use_default_debug_messenger().build();
    if (!inst_ret) std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
    vkb_inst = inst_ret.value();

    if (glfwCreateWindowSurface(vkb_inst, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");

    vkb::PhysicalDeviceSelector device_selector{vkb_inst,surface};
    auto phys_ret = device_selector
            .set_minimum_version(1, 1)
            .select_devices();
    if (!phys_ret) std::cerr << "Failed to select any Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";

    auto phys_device_candidates = phys_ret.value();
    auto main_phys_device = phys_device_candidates[0];
    std::vector<vkb::CustomQueueDescription> main_device_custom_queue_descriptions;

    // Command buffers from the pool can only be submitted on queues corresponding to same queue family.

    // When creating VkImage and VkBuffer, a set of queue families is included to specify the queue families that
    // can access the resource.

    // When inserting a VkBufferMemoryBarrier or VkImageMemoryBarrier,
    // a source and destination queue family index is specified to allow the ownership of a buffer or image to be
    // transferred from one queue family to another.

    for(const auto & qf : main_phys_device.get_queue_families()){
        //todo
    }

    main_device_custom_queue_descriptions.emplace_back(0,1,std::vector<float>{1.0f});
    main_device_custom_queue_descriptions.emplace_back(1,1,std::vector<float>{1.0f});
    main_device_custom_queue_descriptions.emplace_back(2,1,std::vector<float>{1.0f});

    vkb::DeviceBuilder device_builder{main_phys_device};
    auto dev_ret = device_builder.custom_queue_setup(main_device_custom_queue_descriptions).build();
    if (!dev_ret) std::cerr << "Failed to create main Vulkan device. Error: " << dev_ret.error().message() << "\n";
    main_device = dev_ret.value();

    vkb::SwapchainBuilder swapchain_builder{main_device};
    auto swap_ret = swapchain_builder.set_old_swapchain(VK_NULL_HANDLE).set_desired_min_image_count(MAX_FRAME_IN_FLIGHT).build();
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
    destroy_device(main_device);
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
        if (vkCreateSemaphore(main_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create image available semaphore!");
        }
        if(vkCreateSemaphore(main_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ){
            throw std::runtime_error("failed to create render finished semaphore!");
        }
        if(vkCreateFence(main_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS){
            throw std::runtime_error("failed to create in flight fence!");
        }
    }
}


