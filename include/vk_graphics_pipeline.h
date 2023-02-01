//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_GRAPHICS_PIPELINE_H
#define PHY_SIM_VK_GRAPHICS_PIPELINE_H

#include "vulkan/vulkan.hpp"
#include "utility.h"
#include "spirv_reflect.h"

struct VulkanPipeline{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    operator VkPipeline() const{
        return pipeline;
    }
    void destroy(VkDevice device){
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
    }
};

class PipelineBuilder
{
public:

    explicit PipelineBuilder(VkDevice device){
        m_device = device;
    }

    VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        {
            SpvReflectShaderModule module;
            auto result = spvReflectCreateShaderModule(code.size(),reinterpret_cast<const uint32_t *>(code.data()),&module);
            uint32_t count = 0;
            result = spvReflectEnumerateDescriptorSets(&module,&count,NULL);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorSet*> sets(count);
            result = spvReflectEnumerateDescriptorSets(&module,&count,sets.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            //std::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
            for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
                const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
                //DescriptorSetLayoutData& layout = set_layouts[i_set];
                //layout.bindings.resize(refl_set.binding_count);
                for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
                    const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);

                    //VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
                    //layout_binding.binding = refl_binding.binding;
                    //layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                    //layout_binding.descriptorCount = 1;
                    for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                        //layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                    }
                    //layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
                }
                //layout.set_number = refl_set.set;
                //layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                //layout.create_info.bindingCount = refl_set.binding_count;
                //layout.create_info.pBindings = layout.bindings.data();
            }

            spvReflectDestroyShaderModule(&module);
        }

        return shaderModule;
    }

    PipelineBuilder& setShader(VkShaderStageFlagBits stageFlag, const std::string& filePath){
        auto ShaderCode = readFile(filePath);

        VkShaderModule shaderModule = createShaderModule(m_device, ShaderCode);
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = stageFlag;
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";
        m_shaderStages.push_back(shaderStageInfo);
        return *this;
    }

    PipelineBuilder& setVertexInputState(){
        auto static bdes = Vertex::getBindingDescription();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bdes;

        auto static ades = Vertex::getAttributeDescriptions();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(ades.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = ades.data();

        return *this;
    }

    PipelineBuilder& setInputAssemblyState(VkPrimitiveTopology topology){
        inputAssembly.topology = topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        return *this;
    }

    PipelineBuilder& setDepthStencilState(){
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};
        return *this;
    }

    PipelineBuilder& setViewportState(VkExtent2D extent2D){

        static VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) extent2D.width;
        viewport.height = (float) extent2D.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        static VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = extent2D;

        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        return *this;
    }

    PipelineBuilder& setRasterizationState(){
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;//shadow map
        return *this;
    }

    PipelineBuilder& setRasterizationState(float depthBiasConstantFactor,float depthBiasSlopeFactor){
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_TRUE;//shadow map
        rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;
        rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
        return *this;
    }

    PipelineBuilder& setMultisampleState(VkSampleCountFlagBits msaaSamples){
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;
        return *this;
    }

    PipelineBuilder& setColorBlendState(){
        static VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        return *this;
    }

    PipelineBuilder& setPipelineLayouts(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts){
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        return *this;
    }

    PipelineBuilder& setRenderpass(VkRenderPass renderPass){
        m_pass = renderPass;
        return *this;
    }

    std::optional<VulkanPipeline> build() const{

        VkGraphicsPipelineCreateInfo pipelineInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = static_cast<uint32_t>(m_shaderStages.size()),
                .pStages = m_shaderStages.data(),
                .pVertexInputState = &vertexInputStateCreateInfo,
                .pInputAssemblyState = &inputAssembly,
                .pRasterizationState = &rasterizer,
                .pDepthStencilState = &depthStencil,
                .pViewportState = &viewportState,
                .pMultisampleState = &multisampling,
                .pColorBlendState = &colorBlending,
                .layout = m_pipelineLayout,
                .renderPass = m_pass,
                .subpass = 0,
        };

        VkPipeline pipline;
        if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipline) != VK_SUCCESS) {
            return {};
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        VulkanPipeline vk_pipeline{pipline,m_pipelineLayout};
        for(const auto &s:m_shaderStages){
            vkDestroyShaderModule(m_device,s.module, nullptr);
        }
        return vk_pipeline;
    }

     PipelineBuilder& reset() {
        m_pipelineLayout={};
        vertexInputStateCreateInfo= {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        viewportState = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        colorBlending = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        pipelineLayoutInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        m_shaderStages = {};
        m_pass={};
        return *this;
    }

private:
    VkDevice m_device;
    VkPipelineLayout m_pipelineLayout;

    VkPipelineVertexInputStateCreateInfo  vertexInputStateCreateInfo= {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    VkPipelineDepthStencilStateCreateInfo depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    VkPipelineViewportStateCreateInfo viewportState = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    VkPipelineRasterizationStateCreateInfo rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    VkPipelineMultisampleStateCreateInfo multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    VkPipelineColorBlendStateCreateInfo colorBlending = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages = {};
    VkRenderPass m_pass;
};

#endif //PHY_SIM_VK_GRAPHICS_PIPELINE_H
