//
// Created by 王泽远 on 2022/7/1.
//
#include "vk_backend.h"
#include <algorithm>
#include "utility.h"
#include "vulkan_image_utils.h"
#include "VkBootstrap.h"
#include "vk_descriptor.h"
#include "vk_renderpass.h"
#include "material.h"

void MyBackend::init_resource() {
    CommandPoolBuilder commandPoolBuilder{vkb_device};

    commandPool = commandPoolBuilder.setQueue(vkb::QueueType::graphics).build().value();
    commandPoolBuilder.setFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandBuffers = commandPool.allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,vkb_swapchain.image_count);

    for(int i=0;i<vkb_swapchain.image_count;++i){
        auto pool = commandPoolBuilder.build().value();
        gui_commandPools[i] = pool;
    }

    for(int i=0;i<vkb_swapchain.image_count;++i){
        auto cb = gui_commandPools[i].allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        gui_commandBuffers.push_back(cb);
    }

    VulkanCommandManager::instance().setDevice(vkb_device);

    meshRender.setDevice(vkb_device);
    quadRender.setDevice(vkb_device);

    std::thread resource_thread([&]{
        std::thread loadmodelthread([&]{
            loadModel(*mesh, "../res/model/R2-D2/R2-D2.obj");
            meshRender.setMesh(mesh);
            quadRender.setMesh(quadMesh);
        });
        Material mat;
        {
            VkSamplerCreateInfo samplerInfo = {
                    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .mipLodBias = 0.0f,
                    .anisotropyEnable = VK_FALSE,
                    .maxAnisotropy = 16,
                    .compareEnable = VK_FALSE,
                    .compareOp = VK_COMPARE_OP_ALWAYS,
                    .minLod = 0.0f,

                    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    .unnormalizedCoordinates = VK_FALSE,
            };
            {
                Texture texture{"../res/texture/R2-D2/R2D2_BaseColor.png"};
                texture.need_mipmap = true;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod =static_cast<float>(texture.m_mipmap_level),
                texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
            {
                Texture texture{"../res/texture/R2-D2/R2D2_Emissive.png"};
                texture.need_mipmap = true;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod = static_cast<float>(texture.m_mipmap_level),
                        texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
            {
                Texture texture{"../res/texture/R2-D2/R2D2_NormalMap.png"};
                texture.need_mipmap = false;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod = static_cast<float>(texture.m_mipmap_level),
                        texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
            {
                Texture texture{"../res/texture/R2-D2/R2D2_Metallic.png"};
                texture.need_mipmap = true;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod = static_cast<float>(texture.m_mipmap_level),
                        texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
            {
                Texture texture{"../res/texture/R2-D2/R2D2_Roughness.png"};
                texture.need_mipmap = true;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod = static_cast<float>(texture.m_mipmap_level),
                        texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
            {
                Texture texture{"../res/texture/brdfLUT.png"};
                texture.need_mipmap = false;
                texture.m_numSamples = VK_SAMPLE_COUNT_1_BIT;
                texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
                texture.m_tiling = VK_IMAGE_TILING_OPTIMAL;
                texture.m_usage = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                    VK_IMAGE_USAGE_SAMPLED_BIT);
                samplerInfo.maxLod = static_cast<float>(texture.m_mipmap_level),
                        texture.m_samplerInfo = samplerInfo;
                mat.addTexture(texture);
            }
        }

        mat.m_shader.vert = "../res/shader/vert.spv";
        mat.m_shader.frag = "../res/shader/frag.spv";

        Material quadMat;
        quadMat.m_shader.vert = "../res/shader/vert.spv";
        quadMat.m_shader.frag = "../res/material/single_color/frag.spv";

        meshRender.setMaterial(mat);
        quadRender.setMaterial(quadMat);

        std::thread createtextureimgthread([&]{
            meshRender.prepareMaterialResource();
            quadRender.prepareMaterialResource();
        });

        resourceManager.setDevice(vkb_device);
        resourceManager.createColorResources(vkb_swapchain.image_format, vkb_swapchain.extent.width,vkb_swapchain.extent.height, msaaSamples);
        resourceManager.createDepthResources(vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);
        resourceManager.createShadowMapResource(2 * vkb_swapchain.extent.width, 2 * vkb_swapchain.extent.height);
        resourceManager.createUniformBuffers(vkb_swapchain.image_count);

        loadmodelthread.join();
        meshRender.prepareMeshResource();
        quadRender.prepareMeshResource();

        mesh->upload_vertex_data_to_device(VulkanDevice{vkb_device});
        createtextureimgthread.join();
    });

    createForwardPass();
    createShadowPass();
    createImGuiPass();

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder{vkb_device};
    auto des_set_layout_ret = descriptorSetLayoutBuilder
            .addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT).build();
    if(!des_set_layout_ret) std::cerr << "Failed to create Vulkan DescriptorSetLayout" << "\n";

    descriptorSetLayout = des_set_layout_ret.value();

    descriptorSetLayoutBuilder.reset();
    des_set_layout_ret = descriptorSetLayoutBuilder.addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT).build();
    if(!des_set_layout_ret) std::cerr << "Failed to create shadow descriptorSetLayout" << "\n";
    shadowDescriptorSetLayout = des_set_layout_ret.value();

    DescriptorPoolBuilder poolBuilder{vkb_device};
    auto count = 10*vkb_swapchain.image_count;
    poolBuilder.setMaxSets(2*count)
    .setPoolsizes({{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,3*count},{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2*count}});
    descriptorPool = poolBuilder.build().value();

    std::vector<VkDescriptorPoolSize> imgui_pool_sizes ={
            { VK_DESCRIPTOR_TYPE_SAMPLER, count },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count }
    };
    poolBuilder.setPoolsizes(imgui_pool_sizes).setMaxSets(count);
    imguiDescriptorPool = poolBuilder.build().value();

    resource_thread.join();

    {
            PipelineBuilder pipelineBuilder{vkb_device};
        {
            auto topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pipelineBuilder.setShader(VK_SHADER_STAGE_VERTEX_BIT, "../res/shader/shadow.vert.spv")
                    .setVertexInputState().setInputAssemblyState(topology).setDepthStencilState()
                    .setViewportState(VkExtent2D{.width = 2 * vkb_swapchain.extent.width, .height = 2 *
                                                                                                    vkb_swapchain.extent.height})
                    .setRasterizationState(0.0f, 0.5f)
                    .setMultisampleState(VK_SAMPLE_COUNT_1_BIT).setPipelineLayouts({shadowDescriptorSetLayout})
                    .setColorBlendState().setRenderpass(shadowPass);

            auto pipeline_ret = pipelineBuilder.build();
            if (!pipeline_ret) std::cerr << "Failed to build Vulkan shadow Pipeline" << "\n";
            shadowPipeline = pipeline_ret.value();
        }
        {
            auto topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pipelineBuilder.reset()
                    .setVertexInputState().setInputAssemblyState(topology).setDepthStencilState()
                    .setViewportState(vkb_swapchain.extent).setRasterizationState()
                    .setMultisampleState(msaaSamples).setColorBlendState().setRenderpass(forwardPass);

            meshRender.buildPipeline(pipelineBuilder,descriptorSetLayout);
            quadRender.buildPipeline(pipelineBuilder,descriptorSetLayout);
        }
    }

    createDescriptorSets(vkb_swapchain.image_count);
    updateDescriptorSets(vkb_swapchain.image_count);

    createFramebuffers(resourceManager.color.m_device_obj.m_image_view,resourceManager.depth.m_device_obj.m_image_view);
    createGUIFramebuffers();
    createShadowFrambuffers(resourceManager.shadowMap.m_device_obj.m_image_view);
    recordCommandBuffers();
}

void MyBackend::createDescriptorSets(uint32_t count) {

    std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);

    descriptorPool.allocateDescriptorSets(layouts, descriptorSets);

    std::vector<VkDescriptorSetLayout> shadowLayouts(count, shadowDescriptorSetLayout);

    descriptorPool.allocateDescriptorSets(shadowLayouts, shadowDescriptorSets);
}

void MyBackend::updateDescriptorSets(uint32_t count) {
    VkDescriptorBufferInfo uniformBufferInfo = {.offset = 0, .range = VK_WHOLE_SIZE};
    VkDescriptorBufferInfo cameraInfo = {.offset = 0, .range = VK_WHOLE_SIZE};

    VkDescriptorImageInfo shadowMapInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = resourceManager.shadowMap.m_device_obj.m_image_view,
            .sampler = resourceManager.shadowMap.m_device_obj.m_sampler
    };

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstBinding = 0;//layout(binding =0)
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstBinding = 1;//layout(binding =1)
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].pBufferInfo = &cameraInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstBinding = 2;//layout(binding = 3)
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].pImageInfo = &shadowMapInfo;

    descriptorWrites[0].dstArrayElement = descriptorWrites[1].dstArrayElement = descriptorWrites[2].dstArrayElement=0;
    descriptorWrites[0].descriptorCount = descriptorWrites[1].descriptorCount = descriptorWrites[2].descriptorCount = 1;

    descriptorWrites[3] = descriptorWrites[0];

    for (size_t i = 0; i < count; i++) {
        uniformBufferInfo.buffer = resourceManager.getUniformBuffers()[i];
        cameraInfo.buffer = resourceManager.getCameraBuffers()[i];
        descriptorWrites[0].dstSet=descriptorWrites[1].dstSet=descriptorWrites[2].dstSet=descriptorSets[i];
        descriptorWrites[3].dstSet = shadowDescriptorSets[i];
        vkUpdateDescriptorSets(vkb_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }
}

void MyBackend::createForwardPass() {
    VkAttachmentDescription colorAttachment = {
            .format = vkb_swapchain.image_format,
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment = {
            .format = findDepthFormat(vkb_device.physical_device),
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription colorAttachmentResolve = {
            .format = vkb_swapchain.image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,//present_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentRef = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorAttachmentResolveRef = {
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = &colorAttachmentResolveRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
    };

    VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    };
    auto builder = RenderPassBuilder{vkb_device};
    forwardPass = builder.addAttachment(colorAttachment).addAttachment(depthAttachment).addAttachment(colorAttachmentResolve)
    .addSubpass(subpass).addDenpendency(dependency).build().value();
}

void MyBackend::createShadowPass() {
    VkAttachmentDescription attachment = {
            .format = findDepthFormat(vkb_device.physical_device),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .flags = 0
    };

    VkAttachmentReference depth_ref={
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .flags = 0,
            .pDepthStencilAttachment = &depth_ref
    };

    auto builder = RenderPassBuilder{vkb_device};
    shadowPass = builder.addAttachment(attachment).addSubpass(subpass).build().value();
}

void MyBackend::createImGuiPass() {
    VkAttachmentDescription attachment ={
            .format = vkb_swapchain.image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment_ref = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref
    };

    VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    auto builder = RenderPassBuilder{vkb_device};
    imGuiPass = builder.addAttachment(attachment).addSubpass(subpass).addDenpendency(dependency).build().value();
}

void MyBackend::recordCommandBuffer(int idx) {
    auto commandBuffer = commandBuffers[idx];
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    recordShadowPass(commandBuffer,idx);
    recordForwardPass(commandBuffer,idx);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void MyBackend::recordCommandBuffers() {
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        //auto commandBuffer = commandBuffers[i];
        recordCommandBuffer(i);
    }
}

void MyBackend::recordShadowPass(VkCommandBuffer commandBuffer, uint32_t des_idx) {

    shadowPass.begin(commandBuffer,VK_SUBPASS_CONTENTS_INLINE,des_idx,{{.depthStencil.depth = 1.0f,.depthStencil.stencil = 0}});
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.layout, 0, 1,
                            &shadowDescriptorSets[des_idx], 0, nullptr);
    VkBuffer vertexBuffers[] = {mesh->m_device_obj.m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh->m_device_obj.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->get_indices_count()), 1, 0, 0, 0);

    VkBuffer quadVertexBuffers[] = {quadMesh->m_device_obj.m_vertexBuffer};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, quadVertexBuffers, offsets);
    vkCmdDraw(commandBuffer,6,1,0,0);
    shadowPass.end(commandBuffer);
}

void MyBackend::recordForwardPass(VkCommandBuffer commandBuffer, uint32_t des_idx) {
    const float a = 0.7297f;

    //bind global descriptor
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshRender.m_material.m_pipeline.layout,
                            0, 1,&descriptorSets[des_idx], 0, nullptr);

    forwardPass.begin(commandBuffer, VK_SUBPASS_CONTENTS_INLINE, des_idx, {{.color = {a, a, a, 1.0f}}, {.depthStencil = {1.0f, 0}}});
    meshRender.record_drawCommand(commandBuffer);
    quadRender.record_drawCommand(commandBuffer);
    forwardPass.end(commandBuffer);
}

void MyBackend::cleanupSwapChain() {

    resourceManager.cleanUpColorResources();
    resourceManager.cleanUpDepthResources();
    resourceManager.cleanUpUniformBuffers();

    destroyAllFrameBuffers();
    
    commandPool.freeBuffers(commandBuffers);
    commandPool.destroy();
    for(int i =0;i<gui_commandBuffers.size();++i){
        gui_commandPools[i].freeBuffers(gui_commandBuffers.data()+i,1);
        gui_commandPools[i].destroy();
    }

    shadowPipeline.destroy(vkb_device);
    destroyDeviceObject(vkDestroyRenderPass, forwardPass);
    destroyDeviceObject(vkDestroyRenderPass, imGuiPass);

    vkb_swapchain.destroy_image_views(vkb_swapchain.get_image_views().value());

    destroyDeviceObject(vkDestroyDescriptorPool, descriptorPool.m_pool);
    destroyDeviceObject(vkDestroyDescriptorPool, imguiDescriptorPool.m_pool);

    destroy_swapchain(vkb_swapchain);
}

void MyBackend::recreateSwapChain() {

    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vkb_device);

    cleanupSwapChain();
    createForwardPass();
    createImGuiPass();

    resourceManager.createColorResources(vkb_swapchain.image_format, vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);
    resourceManager.createDepthResources(vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);

    createFramebuffers(resourceManager.color.m_device_obj.m_image_view,resourceManager.depth.m_device_obj.m_image_view);
    createGUIFramebuffers();
    createShadowFrambuffers(resourceManager.shadowMap.m_device_obj.m_image_view);

    resourceManager.createUniformBuffers(vkb_swapchain.image_count);

    createDescriptorSets(vkb_swapchain.image_count);
    updateDescriptorSets(vkb_swapchain.image_count);

    recordCommandBuffers();
}

void MyBackend::clean_resource() {
    cleanupSwapChain();
    destroyDeviceObject(vkDestroyDescriptorSetLayout, descriptorSetLayout);
    destroyDeviceObject(vkDestroyDescriptorSetLayout,shadowDescriptorSetLayout);
    meshRender.cleanUp();
    quadRender.cleanUp();
}