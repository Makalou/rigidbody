//
// Created by 王泽远 on 2022/7/10.
//

#ifndef RIGIDBODY_VK_RENDERPASS_H
#define RIGIDBODY_VK_RENDERPASS_H

#include "vulkan/vulkan.hpp"
#include <vector>
#include <optional>

struct VulkanFrameBuffer{
    VkFramebuffer m_frame_buffer;
    VkExtent2D extent;
};

class VulkanRenderPass{
public:

    operator VkRenderPass () const{
        return m_renderPass;
    }
    void addFrameBuffers(uint32_t count,VkExtent2D extent,std::vector<VkImageView> attachments){
        for (size_t i = 0; i < count; i++) {
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = m_renderPass,
                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                    .pAttachments = attachments.data(),
                    .width = extent.width,
                    .height = extent.height,
                    .layers = 1
            };
            VulkanFrameBuffer framebuffer;
            if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer.m_frame_buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
            framebuffer.extent = extent;
            m_framebuffers.push_back(framebuffer);
        }
    }

    void destroyAllFrameBuffers(){
        for (auto framebuffer: m_framebuffers) {
            vkDestroyFramebuffer(m_device,framebuffer.m_frame_buffer, nullptr);
        }
        m_framebuffers.clear();
    }

    VkRenderPass m_renderPass;
    std::vector<VulkanFrameBuffer> m_framebuffers;
    VkDevice m_device;

    void begin(VkCommandBuffer& commandBuffer, VkSubpassContents contents,uint32_t fb_idx,std::vector<VkClearValue> clearValues){
        VkRenderPassBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = m_renderPass,
                .framebuffer = m_framebuffers[fb_idx].m_frame_buffer,
                .renderArea.offset = {0, 0},
                .renderArea.extent = m_framebuffers[fb_idx].extent,
                .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                .pClearValues = clearValues.data()
        };
        vkCmdBeginRenderPass(commandBuffer,&beginInfo,contents);
    }

    void end(VkCommandBuffer& commandBuffer){
        vkCmdEndRenderPass(commandBuffer);
    }
};

class RenderPassBuilder{
public:
    explicit RenderPassBuilder(VkDevice device){
        m_device = device;
    }

    RenderPassBuilder& addAttachment(VkAttachmentDescription attachment){
        attachments.push_back(attachment);
        return *this;
    }

    RenderPassBuilder& addSubpass(VkSubpassDescription subpass){
        subpasses.push_back(subpass);
        return *this;
    }

    RenderPassBuilder& addDenpendency(VkSubpassDependency dependency){
        dependencies.push_back(dependency);
        return *this;
    }

    RenderPassBuilder& reset(){
        attachments.clear();
        subpasses.clear();
        dependencies.clear();
        return *this;
    }
    std::optional<VulkanRenderPass> build(){
        VkRenderPassCreateInfo renderPassInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = static_cast<uint32_t>(subpasses.size()),
                .pSubpasses = subpasses.data(),
                .dependencyCount = static_cast<uint32_t>(dependencies.size()),
                .pDependencies = dependencies.data()
        };

        VkRenderPass renderPass;
        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            return {};
            //throw std::runtime_error("failed to create render pass!");
        }

        VulkanRenderPass vk_renderPass;
        vk_renderPass.m_device = m_device;
        vk_renderPass.m_renderPass = renderPass;
        return vk_renderPass;
    }
private:
    VkDevice m_device;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
};

#endif //RIGIDBODY_VK_RENDERPASS_H
