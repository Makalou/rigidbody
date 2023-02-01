//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VULKAN_IMAGE_UTILS_H
#define PHY_SIM_VULKAN_IMAGE_UTILS_H

#include "vulkan/vulkan.hpp"
VkFormat findSupportedFormat(VkPhysicalDevice phy_dev,const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                            VkFormatFeatureFlags features) {
    for (const auto format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(phy_dev, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }


    throw std::runtime_error("failed to find supported format!");
}
VkFormat findDepthFormat(VkPhysicalDevice phy_dev) {
    return findSupportedFormat(phy_dev,{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

#endif //PHY_SIM_VULKAN_IMAGE_UTILS_H
