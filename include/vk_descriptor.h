//
// Created by 王泽远 on 2022/6/30.
//

#ifndef PHY_SIM_VK_DESCRIPTOR_H
#define PHY_SIM_VK_DESCRIPTOR_H
#include "vulkan/vulkan.hpp"
#include <optional>

class VulkanDescriptorPool{
public:
    void allocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts,std::vector<VkDescriptorSet>& descriptorSets){
        auto count = static_cast<uint32_t>(layouts.size());
        VkDescriptorSetAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = m_pool,
                .descriptorSetCount = count,
                .pSetLayouts = layouts.data()
        };
        descriptorSets.resize(count);
        if (vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }
    VkDescriptorPool m_pool;
    VkDevice m_device;
};

class DescriptorPoolBuilder{
public:
    explicit DescriptorPoolBuilder(VkDevice device){
        m_device = device;
    }
    DescriptorPoolBuilder& setPoolsizes(const std::vector<VkDescriptorPoolSize> poolSizes){
        pPoolSizes = poolSizes;
        return *this;
    }

    DescriptorPoolBuilder& setMaxSets(uint32_t max_set){
        max_set_count = max_set;
        return *this;
    }
    std::optional<VulkanDescriptorPool> build() const{
        VkDescriptorPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .poolSizeCount = static_cast<uint32_t>(std::size(pPoolSizes)),
                .pPoolSizes = pPoolSizes.data(),
                .maxSets = static_cast<uint32_t>(max_set_count)
        };
        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            return {};
        }
        VulkanDescriptorPool vk_pool;
        vk_pool.m_pool = pool;
        vk_pool.m_device = m_device;
        return vk_pool;
    }
private:
    VkDevice m_device;
    std::vector<VkDescriptorPoolSize> pPoolSizes;
    uint32_t max_set_count;
};

class DescriptorSetLayoutBuilder{
public:

    explicit DescriptorSetLayoutBuilder(VkDevice device){
        m_device = device;
    }

    std::optional<VkDescriptorSetLayout> build() const{
        VkDescriptorSetLayoutCreateInfo info={
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data()
        };

        VkDescriptorSetLayout descriptorSetLayout={};
        if (vkCreateDescriptorSetLayout(m_device, &info, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            return {};
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return {descriptorSetLayout};
    }

    DescriptorSetLayoutBuilder& addBinding(VkDescriptorType type,uint32_t count,VkShaderStageFlags flags){
        uint32_t cur_size = bindings.size();
        bindings.push_back({
                                      .binding = cur_size,
                                      .descriptorType = type,
                                      .descriptorCount = count,
                                      .stageFlags = flags,
                                      .pImmutableSamplers = nullptr
        });
        return *this;
    }

    DescriptorSetLayoutBuilder& reset(){
        bindings.clear();
        return *this;
    }
private:
    VkDevice m_device;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};
#endif //PHY_SIM_VK_DESCRIPTOR_H
