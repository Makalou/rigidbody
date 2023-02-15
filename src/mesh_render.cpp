//
// Created by 王泽远 on 2023/2/10.
//

#include "mesh_render.h"

void MeshRender::record_drawCommand(VkCommandBuffer commandBuffer) {
    m_material.bindPipeline(commandBuffer); //todo : batching
    m_material.bindDescriptorSet(commandBuffer); // todo : batching
    VkDeviceSize offsets[] = {0};
    VkBuffer vertexBuffers[] = {m_mesh->m_device_obj.m_vertexBuffer};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    //todo : how to support instancing?
    if(m_mesh->indices_buffer_size()>0){
        vkCmdBindIndexBuffer(commandBuffer, m_mesh->m_device_obj.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        //In shader glInstanceIdx : [firstInstance,firstInstance + instanceCount]
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_mesh->get_indices_count()), 1, 0, 0, 0);
    }
    else{
        vkCmdDraw(commandBuffer,m_mesh->get_vertex_size(),1,0,0);
    }
}

void MeshRender::prepareMeshResource() const {
    m_mesh->create_device_obj(*m_vulkanDevice);
    m_mesh->upload_vertex_data_to_device(*m_vulkanDevice);
    m_mesh->upload_indices_data_to_device(*m_vulkanDevice);
}

void MeshRender::prepareMaterialResource() {
    m_material.setDevice(m_vulkanDevice);
    m_material.prepareResource();
    m_material.init_descriptorPool();
    m_material.createDescriptorSetLayout();
    m_material.allocateDescriptorSets();
    m_material.updateDescriptorSet();
}