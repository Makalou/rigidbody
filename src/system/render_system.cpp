//
// Created by 王泽远 on 2023/2/11.
//

#include "system/render_system.h"

namespace subsystem {

    void RenderSystem::renderGUI(VkCommandBuffer commandBuffer, uint32_t index) {
        VkCommandBufferBeginInfo cmd_begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        if (vkBeginCommandBuffer(commandBuffer, &cmd_begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo pass_begin_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = gpu_backend->imGuiPass,
                .framebuffer = gpu_backend->imGuiPass.m_framebuffers[index].m_frame_buffer,
                .renderArea.extent.width = gpu_backend->vkb_swapchain.extent.width,
                .renderArea.extent.height = gpu_backend->vkb_swapchain.extent.height
        };

        vkCmdBeginRenderPass(commandBuffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer);
    }

    void RenderSystem::drawFrame(int currentFrame, bool *frameBufferResized, bool rota) {
        vkWaitForFences(gpu_backend->vkb_device, 1, &gpu_backend->inFlightFences[currentFrame], VK_TRUE,
                        std::numeric_limits<uint64_t>::max());
        uint32_t imageIndex;
        auto acquire_result = vkAcquireNextImageKHR(gpu_backend->vkb_device, gpu_backend->get_swapChain(),
                                                    std::numeric_limits<uint64_t>::max(),
                                                    gpu_backend->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                                                    &imageIndex);

        if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || *frameBufferResized) {
            *frameBufferResized = false;
            gpu_backend->recreateSwapChain();
            return;
        } else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        auto aspect = gpu_backend->vkb_swapchain.extent.width / (float) gpu_backend->vkb_swapchain.extent.height;
        gpu_backend->resourceManager.updateUniformBuffer(imageIndex, rota);
        gpu_backend->resourceManager.updateCameraBuffer(imageIndex);//todo should move to other place
        //gpu_backend->resourceManager.updateModelMatrix(imageIndex,aspect,x,q);


        auto command = gpu_backend->commandBuffers[imageIndex];
        auto gui_command = gpu_backend->gui_commandBuffers[imageIndex];

        renderGUI(gui_command, imageIndex);

        VkSemaphore signalSemaphores[] = {gpu_backend->renderFinishedSemaphores[currentFrame]};

        VkCommandBuffer commandBuffers[] = {command, gui_command};

        VkSemaphore waitSemaphores[] = {gpu_backend->imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphores,
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = std::size(commandBuffers),
                .pCommandBuffers = commandBuffers,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphores
        };

        //render queue -> producer
        //present queue -> consumer
        //swapchain -> mailbox
        vkResetFences(gpu_backend->vkb_device, 1, &gpu_backend->inFlightFences[currentFrame]);

        if (vkQueueSubmit(gpu_backend->vkb_device.get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo,
                          gpu_backend->inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = {gpu_backend->get_swapChain()};
        VkPresentInfoKHR presentInfo = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = signalSemaphores,
                .swapchainCount = 1,
                .pSwapchains = swapChains,
                .pImageIndices = &imageIndex
        };

        auto qp_result = vkQueuePresentKHR(gpu_backend->vkb_device.get_queue(vkb::QueueType::present).value(), &presentInfo);
        if (qp_result == VK_ERROR_OUT_OF_DATE_KHR || qp_result == VK_SUBOPTIMAL_KHR || *frameBufferResized) {
            *frameBufferResized = false;
            gpu_backend->recreateSwapChain();
        } else if (qp_result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}
