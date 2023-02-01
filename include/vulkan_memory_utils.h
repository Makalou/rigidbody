//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VULKAN_MEMORY_UTILS_H
#define PHY_SIM_VULKAN_MEMORY_UTILS_H

#include <cstdint>
#include <vulkan/vulkan.h>
#include <stdexcept>

uint32_t static findMemoryType(VkPhysicalDevice phy_device, uint32_t typeFliter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(phy_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFliter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

#endif //PHY_SIM_VULKAN_MEMORY_UTILS_H
