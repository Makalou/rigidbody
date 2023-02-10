//
// Created by 王泽远 on 2022/7/6.
//

#ifndef RIGIDBODY_TEXTURE_H
#define RIGIDBODY_TEXTURE_H

#include <stdexcept>
#include "stb_image.h"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include "vk_device.h"
#include <cmath>
//template<TexureDim dim>
class Texture{
public:
   explicit Texture()= default;

   explicit Texture(const std::string& path){
       m_path = path;
   }

   void load(){
       if(m_path.empty()) return;
       loadFromFile(m_path.c_str());
   }

    void free_from_main_memory(){
        stbi_image_free(mp_pixels);
    }

    void free_from_device(const VkDevice &device){
        if(m_device_obj.m_sampler)
            vkDestroySampler(device,m_device_obj.m_sampler, nullptr);
        vkDestroyImageView(device,m_device_obj.m_image_view, nullptr);
        vkDestroyImage(device,m_device_obj.m_image, nullptr);
        vkFreeMemory(device,m_device_obj.m_deviceMemory, nullptr);
    }

    void upload_to_device(const VulkanDevice& device){
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};

        device.createBuffer(m_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                                    stagingBufferMemory);
        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, m_image_size, 0, &data);
        memcpy(data, mp_pixels, static_cast<size_t>(m_image_size));
        vkUnmapMemory(device, stagingBufferMemory);
        //now texture data is on GPU,then we need transfer from buffer to image
        transitionImageLayoutTo( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, m_device_obj.m_image, m_width, m_height);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    static void
    copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
    }

    void create_device_obj(const vkb::Device &device){
        createImage(device,m_width, m_height, m_mipmap_level, m_numSamples, m_format,
                    m_tiling,
                    m_usage,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_device_obj.m_image,m_device_obj.m_deviceMemory);
        m_device_obj.m_image_view = createImageView(device,m_device_obj.m_image, m_format, m_aspect,m_mipmap_level);

        if(need_sampler) {
            if (vkCreateSampler(device, &m_samplerInfo, nullptr, &m_device_obj.m_sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }

    void static
    createImage(const vkb::Device& device,uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                VkImage &image, VkDeviceMemory &imageMemory) {
        VkImageCreateInfo imageInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .extent.width = width,
                .extent.height = height,
                .extent.depth = 1,
                .mipLevels = mipLevels,
                .arrayLayers = 1,
                .format = format,
                .tiling = tiling,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .samples = numSamples,
                .flags = 0,//for sparse image

        };

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);
        VkMemoryAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = findMemoryType(device.physical_device, memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkImageView
    createImageView(const VkDevice & device,VkImage image, VkFormat format, VkImageAspectFlags aspectflags, uint32_t mipLevels) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectflags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageview;
        if (vkCreateImageView(device, &createInfo, nullptr, &imageview) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageview;
    }

    static VkFormat
    findSupportedFormat(const vkb::Device& device,const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (const auto format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device.physical_device, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    static VkFormat
    findDepthFormat(const vkb::Device& device) {
        return findSupportedFormat(device,{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    void generateMipmaps() {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .image = m_device_obj.m_image,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
                .subresourceRange.levelCount = 1,
        };


        int32_t mipWidth = m_width;
        int32_t mipHeight = m_height;
        for (uint32_t i = 1; i < m_mipmap_level; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit = {
                    .srcOffsets[0] = {0, 0, 0},
                    .srcOffsets[1] = {mipWidth, mipHeight, 1},
                    .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .srcSubresource.mipLevel = i - 1,
                    .srcSubresource.baseArrayLayer = 0,
                    .srcSubresource.layerCount = 1,

                    .dstOffsets[0] = {0, 0, 0},
                    .dstOffsets[1] = {mipWidth > 1 ? (mipWidth / 2) : 1, mipHeight > 1 ? (mipHeight / 2) : 1, 1},
                    .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .dstSubresource.mipLevel = i,
                    .dstSubresource.baseArrayLayer = 0,
                    .dstSubresource.layerCount = 1
            };

            vkCmdBlitImage(commandBuffer,
                           m_device_obj.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_device_obj.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier
            );
            if (mipWidth > 1)mipWidth /= 2;
            if (mipHeight > 1)mipHeight /= 2;

        }

        barrier.subresourceRange.baseMipLevel = m_mipmap_level - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier
        );

        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
    }

    static bool hasStencilComponenet(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void transitionImageLayoutTo(VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = m_layout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_device_obj.m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_mipmap_level;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if ((m_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if ((m_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) &&(newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if ((m_layout == VK_IMAGE_LAYOUT_UNDEFINED) &&(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponenet(m_format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else if ((m_layout == VK_IMAGE_LAYOUT_UNDEFINED) && newLayout == (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else {
            throw std::runtime_error("unsupported layout transition!");
        }
        vkCmdPipelineBarrier(commandBuffer,
                             srcStage, dstStage, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
        m_layout = newLayout;
    }
private:
    void loadFromFile(const char* filename){
        mp_pixels = stbi_load(filename, &m_width, &m_height, &m_channels,
                              STBI_rgb_alpha);
        if (!mp_pixels) {
            throw std::runtime_error("failed to load image!");
        }
        m_image_size = m_width*m_height*4;
        if(need_mipmap)
            m_mipmap_level = static_cast<uint32_t>(std::floor(std::log2(std::max(m_width,m_height)))) + 1;
    }
public:
    std::string m_path;
    stbi_uc * mp_pixels;
    int m_width,m_height,m_channels,m_image_size;
    uint32_t m_mipmap_level=1;
    VkSampleCountFlagBits m_numSamples = VK_SAMPLE_COUNT_1_BIT;
    VkFormat m_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageTiling m_tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlagBits m_usage;
    VkImageAspectFlagBits m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

    struct DeviceObject{
        VkDeviceMemory m_deviceMemory;
        VkImage m_image;
        VkImageView m_image_view;
        VkSampler m_sampler;
    }m_device_obj;
    bool need_sampler = true;
    bool need_mipmap = false;
    VkSamplerCreateInfo m_samplerInfo;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class Texture2D:public Texture{

};

class TextureCube:public Texture{

};
/*
 *
enum class TexureDim{
    D2,
    D3,
};

template<>
class Texture<TexureDim::D2>{

};

template<>
class Texture<TexureDim::D3>{

};
 */

#endif //RIGIDBODY_TEXTURE_H
