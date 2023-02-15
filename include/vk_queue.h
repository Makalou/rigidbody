//
// Created by 王泽远 on 2023/2/13.
//

#ifndef RIGIDBODY_VK_QUEUE_H
#define RIGIDBODY_VK_QUEUE_H

#include "vulkan/vulkan.hpp"

struct VulkanQueue{
    VkQueue queue;
    uint32_t queueFamilyIdx;

    VulkanQueue(VkQueue q){
        queue = q;
    }
    // Command buffers from the pool can only be submitted on queues corresponding to same queue family.
    std::vector<VkSubmitInfo> submit_batches;

    void addSubmitBatch(const std::vector<VkCommandBuffer>& commandBuffers,
                        const std::vector<VkSemaphore>& waitSemaphores,
                        const std::vector<VkPipelineStageFlags>& waitDstStageMasks,
                        const std::vector<VkSemaphore>& signalSemaphores){
        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
            .pCommandBuffers = commandBuffers.data(),
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitDstStageMasks.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data()
        };
        submit_batches.push_back(info);
    }

    void submitCurrentBatches(VkFence fence,bool reset = true){
        vkQueueSubmit(queue,submit_batches.size(),submit_batches.data(),fence);
        if(reset){
            submit_batches.clear();
        }
    }
};
#endif //RIGIDBODY_VK_QUEUE_H
