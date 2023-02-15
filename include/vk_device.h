//
// Created by 王泽远 on 2022/7/1.
//

#ifndef PHY_SIM_VK_DEVICE_H
#define PHY_SIM_VK_DEVICE_H

#include "vk_command_manager.h"
#include "vulkan_memory_utils.h"

class VulkanQueue;

class VulkanDevice{
public:
    operator vkb::Device() const{
        return m_device;
    }
    operator VkDevice() const{
        return m_device;
    }

    VulkanDevice(){

    }
    explicit VulkanDevice(vkb::Device device){
        m_device = device;
    }

    void setDevice(const vkb::Device& device){
        m_device = device;
    }

    auto get_queue_by_type(vkb::QueueType type) const{
        return m_device.get_queue(type).value();
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,VkDeviceMemory &bufferMemory) const{
        VkBufferCreateInfo bufferInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = size,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);
        VkMemoryAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,//the size of the allocation in bytes
                .memoryTypeIndex = findMemoryType(m_device.physical_device, memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }

    static void
    copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandbuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandbuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandbuffer);
    }

    void updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes){
        vkUpdateDescriptorSets(m_device,writes.size(),writes.data(),0,VK_NULL_HANDLE);
    }

    void destroy(){
        vkDestroyDevice(m_device, nullptr);
    }

    VkPhysicalDevice getPhysicalDevice()const {
        return m_device.physical_device;
    }

    auto get_queue(vkb::QueueType queueType) const{
        return m_device.get_queue(queueType);
    }

private:
    vkb::Device m_device;
};

class VulkanQueue{
public:
    void new_command_pool() {
        CommandPoolBuilder builder{m_device};
        auto pool = builder.setQueue(m_type).build().value();
        m_command_pools.push_back(pool);
    }
    auto get_command_pools() {
        return m_command_pools;
    }
private:
    vkb::Device m_device;
    vkb::QueueType m_type;
    std::vector<VulkanCommandPool> m_command_pools;
};
#endif //PHY_SIM_VK_DEVICE_H
