//
// Created by 王泽远 on 2022/5/26.
//
#include "vk_resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"


void VulkanResourceManager::createUniformBuffers(size_t ub_size) {
    VkDeviceSize buffersize = sizeof(UniformBufferObject);
    uniformBuffers.resize(ub_size);
    uniformBuffersMemory.resize(ub_size);
    for (size_t i = 0; i < ub_size; i++) {
        m_vulkanDevice.createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
                     uniformBuffersMemory[i]);
    }
    buffersize = sizeof(CameraInfo);
    cameraBuffers.resize(ub_size);
    cameraBuffersMemory.resize(ub_size);
    for (size_t i = 0; i < ub_size; i++) {
        m_vulkanDevice.createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, cameraBuffers[i],
                     cameraBuffersMemory[i]);
    }
}

void VulkanResourceManager::updateUniformBuffer(uint32_t currentImage,bool rotate) {
    UniformBufferObject ubo = {};
    UniformBufferObject::update(ubo,rotate);
    void *data;
    vkMapMemory(m_device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device, uniformBuffersMemory[currentImage]);
}

void VulkanResourceManager::updateCameraBuffer(uint32_t currentImage) {
    auto cam_info = Camera::getCamera("camera1")->get_camera_info();
    void *data;
    vkMapMemory(m_device, cameraBuffersMemory[currentImage], 0, sizeof(cam_info), 0,&data);
    memcpy(data, &cam_info, sizeof(cam_info));
    vkUnmapMemory(m_device, cameraBuffersMemory[currentImage]);
}

void VulkanResourceManager::createColorResources(VkFormat swapChainImageFormat, uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples) {
    color.m_width = width;
    color.m_height = height;
    color.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    color.m_format = swapChainImageFormat;
    color.m_numSamples = msaaSamples;
    color.need_sampler = false;
    color.create_device_obj(m_device);
    color.transitionImageLayoutTo(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanResourceManager::createDepthResources(uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples) {
    depth.m_width = width;
    depth.m_height = height;
    depth.m_numSamples = msaaSamples;
    depth.m_format = Texture::findDepthFormat(m_device);
    depth.m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth.m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth.need_sampler = false;
    depth.create_device_obj(m_device);
    depth.transitionImageLayoutTo(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanResourceManager::createShadowMapResource(uint32_t width,uint32_t height) {

    shadowMap.m_width = width;
    shadowMap.m_height = height;
    shadowMap.m_format = Texture::findDepthFormat(m_device);
    shadowMap.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    shadowMap.m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    shadowMap.need_mipmap = false;
    shadowMap.need_sampler = true;

    VkSamplerCreateInfo shadowMapSamplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .anisotropyEnable = false,
            .maxAnisotropy = 1.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 100.0f,
    };
    shadowMap.m_samplerInfo = shadowMapSamplerInfo;
    shadowMap.create_device_obj(m_device);
    shadowMap.transitionImageLayoutTo(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
