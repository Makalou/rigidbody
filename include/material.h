//
// Created by 王泽远 on 2022/7/6.
//

#ifndef RIGIDBODY_MATERIAL_H
#define RIGIDBODY_MATERIAL_H

#include <memory>
#include <vector>

#include "texture.h"
#include "glm/glm.hpp"
#include "vk_descriptor.h"
#include "vk_device.h"
#include "tiny_obj_loader.h"
#include "vulkan/vulkan.hpp"
#include "vk_graphics_pipeline.h"

struct ShaderProgram{
    std::string vert;
    std::string geom;
    std::string tes;
    std::string frag;
};

class CustomShaderParam{};

class Material{
public:
    ShaderProgram m_shader;

    std::shared_ptr<CustomShaderParam> m_uniform_buffer;

    std::vector<Texture> m_textures;

    VulkanDescriptorPool m_descriptor_pool;
    VkDescriptorSet m_descriptor_set;
    VkDescriptorSetLayout m_layout;

    VulkanPipeline m_pipeline;

    //VkDevice m_device;
    VulkanDevice* m_device;

    void setCustomUniformBuffer(const std::shared_ptr<CustomShaderParam>& buffer){
        m_uniform_buffer = buffer;
    }

    void addTexture(const Texture& tex){
        m_textures.push_back(tex);
    }

    void setDevice(VulkanDevice* device){
        m_device = device;
    }

    void prepareResource(){
        for(auto &tex:m_textures){
            tex.load();
            tex.create_device_obj(*m_device);
            tex.upload_to_device(*m_device);
            tex.generateMipmaps();
            tex.free_from_main_memory();
        }
    }

    void init_descriptorPool(){
        DescriptorPoolBuilder builder{*m_device};
        builder.setMaxSets(10);
        if(!m_uniform_buffer && m_textures.empty()) return;
        if(m_textures.size()==0){
            builder.setPoolsizes({{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1}});
        }else{
            builder.setPoolsizes({{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1},
                           {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,static_cast<uint32_t>(m_textures.size())}});
        }
        m_descriptor_pool = builder.build().value();
    }

    void createDescriptorSetLayout(){
        if(!m_uniform_buffer && m_textures.empty()) return;
        DescriptorSetLayoutBuilder layoutBuilder{*m_device};
        if(m_uniform_buffer)
            layoutBuilder.addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        for(int i=0;i<m_textures.size();++i)
            layoutBuilder.addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
       m_layout = layoutBuilder.build().value();
    }

    void allocateDescriptorSets(){
        if(m_layout)
            m_descriptor_pool.allocateDescriptorSet(m_layout, m_descriptor_set);
    }

    void buildPipeline(PipelineBuilder builder,const VkDescriptorSetLayout& global_dscptor_set_layout){
        if(!m_shader.vert.empty()) builder.setShader(VK_SHADER_STAGE_VERTEX_BIT,m_shader.vert);
        if(!m_shader.geom.empty()) builder.setShader(VK_SHADER_STAGE_GEOMETRY_BIT,m_shader.geom);
        if(!m_shader.tes.empty()) builder.setShader(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,m_shader.tes);
        if(!m_shader.frag.empty()) builder.setShader(VK_SHADER_STAGE_FRAGMENT_BIT,m_shader.frag);
        if(m_layout)
            builder.setPipelineLayouts({global_dscptor_set_layout,m_layout});
        else
            builder.setPipelineLayouts({global_dscptor_set_layout});
        m_pipeline = builder.build().value();
    }

    void bindPipeline(VkCommandBuffer& commandBuffer){
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    void updateDescriptorSet(){
        std::vector<VkDescriptorImageInfo> descriptorImageInfos;
        descriptorImageInfos.reserve(m_textures.size());
        for(auto & m_texture : m_textures){
            descriptorImageInfos.push_back({
                .sampler = m_texture.m_device_obj.m_sampler,
                .imageView = m_texture.m_device_obj.m_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(m_textures.size());
        for(int i=0; i < m_textures.size(); ++i){
            descriptorWrites.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_set,
                .dstBinding = static_cast<uint32_t>(i),
                .dstArrayElement = 0,
                .descriptorCount =1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &descriptorImageInfos[i],
                });
        }

        if(descriptorWrites.empty()) return;
        m_device->updateDescriptorSets(descriptorWrites);
    }

    void bindDescriptorSet(VkCommandBuffer& commandBuffer){
        if(!m_descriptor_set) return;
        vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipeline.layout,
                                1, 1,&m_descriptor_set,
                                0, nullptr);
    }

    void destroyObj(){
        for(auto & tex: m_textures)
            tex.free_from_device(*m_device);
        m_pipeline.destroy(*m_device);
    }
};

#endif //RIGIDBODY_MATERIAL_H
