//
// Created by 王泽远 on 2022/7/6.
//

#ifndef RIGIDBODY_MESH_RENDER_H
#define RIGIDBODY_MESH_RENDER_H

#include "material.h"
#include "mesh.h"
#include "vk_graphics_pipeline.h"

class MeshRender{
public:
    enum class ShadowMode{
        OFF,
        ON,
        TWO_SIDE,
        SHADOWS_ONLY
    };

    void setDevice(vkb::Device device){
        m_vulkanDevice.setDevice(device);
    }

    void record_drawCommand(VkCommandBuffer commandBuffer){
        for(auto& m : m_materials){
            m.bindPipeline(commandBuffer);
            m.bindDescriptorSet(commandBuffer);
            VkDeviceSize offsets[] = {0};
            VkBuffer vertexBuffers[] = {m_mesh->m_device_obj.m_vertexBuffer};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            if(m_mesh->indices_buffer_size()>0){
                vkCmdBindIndexBuffer(commandBuffer, m_mesh->m_device_obj.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_mesh->get_indices_count()), 1, 0, 0, 0);
            }
            else{
                vkCmdDraw(commandBuffer,m_mesh->get_vertex_size(),1,0,0);
            }
        }
    }

    void prepareMeshResource(){
        m_mesh->create_device_obj(m_vulkanDevice);
        m_mesh->upload_vertex_data_to_device(m_vulkanDevice);
        m_mesh->upload_indices_data_to_device(m_vulkanDevice);
    }

    void prepareMaterialResource(){
        for(auto& mat: m_materials){
            for(auto &tex:mat.m_textures){
                tex.load();
                tex.create_device_obj(m_vulkanDevice);
                tex.upload_to_device(m_vulkanDevice);
                tex.generateMipmaps();
                tex.free_from_main_memory();
            }
            mat.init_descriptorPool(m_vulkanDevice);
            mat.createDescriptorSetLayout();
            mat.allocateDescriptorSets();
        }
        updateDescriptorSets();
    }

    void buildPipeline(PipelineBuilder builder,const VkDescriptorSetLayout& global_dscptor_set_layout){
        for(auto& mat: m_materials){
          mat.buildPipeline(builder,global_dscptor_set_layout);
        }
    }

    void updateDescriptorSets(){
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        for(auto& mat:m_materials){
            for(int i=0;i<mat.m_textures.size();++i){
                VkDescriptorImageInfo* pImageInfo= new VkDescriptorImageInfo({.sampler = mat.m_textures[i].m_device_obj.m_sampler,
                                    .imageView = mat.m_textures[i].m_device_obj.m_image_view,
                                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstBinding = i;
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.dstArrayElement = 0;
                write.descriptorCount =1;
                write.pImageInfo = pImageInfo;
                write.dstSet = mat.m_dscptor_sets[0];

                descriptorWrites.push_back(write);
            }
        }

        if(descriptorWrites.empty()) return;
        vkUpdateDescriptorSets(m_vulkanDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }

    void setMesh(const std::shared_ptr<Mesh>& mesh){
        m_mesh = mesh;
    }

    void addMaterial(const Material& material){
        m_materials.push_back(material);
    }

    void cleanUp()
    {
        m_mesh->free_from_device(m_vulkanDevice);
        for(auto & mat : m_materials)
            mat.destroyObj();
    }

public:
    std::shared_ptr<Mesh> m_mesh;
    std::vector<Material> m_materials;
    ShadowMode m_shadowMode = ShadowMode::ON;
    VulkanDevice m_vulkanDevice;
};
#endif //RIGIDBODY_MESH_RENDER_H
