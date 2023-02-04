//
// Created by 王泽远 on 2023/2/1.
//

#ifndef RIGIDBODY_SCENE_H
#define RIGIDBODY_SCENE_H

#include <vector>
#include <vulkan/vulkan.hpp>

struct SceneRenderingTableCPU;

struct SceneGraph{
    void update(float deltaTime){

    }
    std::unique_ptr<SceneRenderingTableCPU> sceneRenderingTableCpu;
};

struct SceneRenderingTableCPU{
    std::vector<int> global_view_independent_params; // time, lights info etc.

    std::vector<int> pipelines;

    std::vector<int> materials;

    std::vector<int> textures;
    std::vector<int> shaders;
    std::vector<int> geometries; // meshes, points(particle) etc.

    std::vector<int> per_view_params; // camera info, skybox(clear value) info etc.
    std::vector<int> per_material_params; // shader param, textures etc.
    std::vector<int> per_obj_params; // model matrix etc. todo How about GPU skinning data?
};

struct SceneRenderingTableGPU{

    //pipelines
    std::vector<VkPipeline> graphics_pipelines;
    std::vector<VkPipeline> compute_pipelines;
    std::vector<VkPipeline> raytracing_pipelines;

    //scene global data (low binding frequency)
    VkBuffer scene_time;
    std::vector<VkBuffer> light_buffers;

    //material data (medium binding frequency)
    std::vector<VkBuffer> shaderParams;
    std::vector<VkImageView> textures;

    //draw data (high binding frequency)
    std::vector<VkBuffer> vertexBuffers;
    std::vector<VkBuffer> indexBuffers;
    std::vector<VkBuffer> instanceBuffers;

    std::vector<VkDescriptorSet> descriptorSets;
};

#endif //RIGIDBODY_SCENE_H
