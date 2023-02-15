//
// Created by 王泽远 on 2023/2/10.
//

#include "mesh_render.h"

void MeshRender::record_drawCommand(VkCommandBuffer commandBuffer) {
    m_material.bindPipeline(commandBuffer);
    m_material.bindDescriptorSet(commandBuffer);
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

void MeshRender::prepareMeshResource() {
    m_mesh->create_device_obj(m_vulkanDevice);
    m_mesh->upload_vertex_data_to_device(m_vulkanDevice);
    m_mesh->upload_indices_data_to_device(m_vulkanDevice);
}

void MeshRender::prepareMaterialResource() {
    for(auto &tex:m_material.m_textures){
        tex.load();
        tex.create_device_obj(m_vulkanDevice);
        tex.upload_to_device(m_vulkanDevice);
        tex.generateMipmaps();
        tex.free_from_main_memory();
    }
    m_material.init_descriptorPool(m_vulkanDevice);
    m_material.createDescriptorSetLayout();
    m_material.allocateDescriptorSets();
    updateDescriptorSets();
}

void MeshRender::updateDescriptorSets() {
    std::vector<VkDescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(m_material.m_textures.size());
    for(auto & m_texture : m_material.m_textures){
        descriptorImageInfos.push_back({
            .sampler = m_texture.m_device_obj.m_sampler,
            .imageView = m_texture.m_device_obj.m_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(m_material.m_textures.size());
    for(int i=0; i < m_material.m_textures.size(); ++i){
        descriptorWrites.push_back({
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_material.m_dscptor_sets[0],
            .dstBinding = static_cast<uint32_t>(i),
            .dstArrayElement = 0,
            .descriptorCount =1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &descriptorImageInfos[i],
        });
    }

    if(descriptorWrites.empty()) return;
    m_vulkanDevice.updateDescriptorSets(descriptorWrites);
}
