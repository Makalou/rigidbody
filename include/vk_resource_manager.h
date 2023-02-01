//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_RESOURCE_MANAGER_H
#define PHY_SIM_VK_RESOURCE_MANAGER_H

#include "vulkan/vulkan.hpp"
#include "vertex.h"
#include "UniformBufferObject.h"
#include "vulkan_memory_utils.h"
#include "vk_command_manager.h"
#include "mesh.h"
#include "texture.h"
#include "vk_device.h"
#include "material.h"

class VulkanResourceManager {
public:
    void setDevice(vkb::Device device){
        m_device = device;
        m_vulkanDevice.setDevice(m_device);
    }

    void createUniformBuffers(size_t ub_size);

    void updateUniformBuffer(uint32_t currentImage,bool rotate);

    void updateCameraBuffer(uint32_t currentImage);

    //void createMaterialResources();

    void createColorResources(VkFormat swapChainImageFormat, uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples);

    void createDepthResources(uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples);

    void createShadowMapResource(uint32_t width,uint32_t height);

    void cleanUpColorResources() {
        color.free_from_device(m_device);
    }

    void cleanUpDepthResources() {
        depth.free_from_device(m_device);
        shadowMap.free_from_device(m_device);
    }

    void cleanUpUniformBuffers() {
        for (auto uniformBuffer: uniformBuffers) {
            vkDestroyBuffer(m_device, uniformBuffer, nullptr);
        }

        for (auto memory: uniformBuffersMemory) {
            vkFreeMemory(m_device, memory, nullptr);
        }
    }

    auto &getUniformBuffers() {
        return uniformBuffers;
    }

    auto &getCameraBuffers(){
        return cameraBuffers;
    }

private:

    vkb::Device m_device;
    VulkanDevice m_vulkanDevice{};
private:
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<VkDeviceMemory> cameraBuffersMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkBuffer> cameraBuffers;
public:
    Texture color;
    Texture depth;
    Texture shadowMap;
};

#endif //PHY_SIM_VK_RESOURCE_MANAGER_H
