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

    void record_drawCommand(VkCommandBuffer commandBuffer);

    void prepareMeshResource();

    void prepareMaterialResource();

    void buildPipeline(PipelineBuilder builder,const VkDescriptorSetLayout& global_dscptor_set_layout){
        m_material.buildPipeline(builder, global_dscptor_set_layout);
    }

    void updateDescriptorSets();

    void setMesh(const std::shared_ptr<Mesh>& mesh){
        m_mesh = mesh;
    }

    void setMaterial(const Material& material){
        m_material = material;
    }

    void cleanUp()
    {
        m_mesh->free_from_device(m_vulkanDevice);
        m_material.destroyObj();
    }

public:
    std::shared_ptr<Mesh> m_mesh;
    Material m_material;
    ShadowMode m_shadowMode = ShadowMode::ON;
    VulkanDevice m_vulkanDevice;
};
#endif //RIGIDBODY_MESH_RENDER_H
