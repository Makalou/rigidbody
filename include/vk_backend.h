#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_resource_manager.h"
#include "vk_graphics_pipeline.h"
#include "vk_command_manager.h"
#include "GLFW/glfw3.h"
#include <vector>

#include "VkBootstrap.h"
#include "vk_descriptor.h"
#include "vk_renderpass.h"
#include "mesh_render.h"

const size_t MAX_FRAME_IN_FLIGHT = 3;

class VulkanBackend {
public:
    void init(GLFWwindow *window);

    void cleanup();

    virtual void recreateSwapChain() = 0;

protected:
    virtual void init_resource() = 0;
    virtual void clean_resource() = 0;
public:

    VkSwapchainKHR get_swapChain() const {
        return vkb_swapchain;
    }

    auto getDevice() const{
        return main_device;
    }

    void createSyncObjects();

    template<typename Func, typename ... Args>
    void destroyDeviceObject(Func f, Args... args){
        f(main_device, args..., nullptr);
    }

public:
    GLFWwindow *window;

    vkb::Instance vkb_inst;
    vkb::Device main_device;
    VkSurfaceKHR surface;
    vkb::Swapchain vkb_swapchain;

    std::array<VkSemaphore,MAX_FRAME_IN_FLIGHT> imageAvailableSemaphores;
    std::array<VkSemaphore,MAX_FRAME_IN_FLIGHT> renderFinishedSemaphores;
    std::array<VkFence,MAX_FRAME_IN_FLIGHT> inFlightFences;
};

class MyBackend: public VulkanBackend{
    void init_resource() override;

    void clean_resource() override;

    void createFramebuffers(VkImageView color,VkImageView depth) {
        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        auto image_views = vkb_swapchain.get_image_views().value();
        auto extent = vkb_swapchain.extent;
        for (size_t i = 0; i < swapChainImageViewsSize; i++) {
            forwardPass.addFrameBuffers(1, extent, {color, depth, image_views[i]});
        }
    }

    void createGUIFramebuffers() {
        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        auto extent = vkb_swapchain.extent;
        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            imGuiPass.addFrameBuffers(1, extent, {vkb_swapchain.get_image_views().value()[i]});
        }
    }

    void createShadowFrambuffers(VkImageView shadowMapView){
        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        VkExtent2D extent = {.width = vkb_swapchain.extent.width * 2,.height = vkb_swapchain.extent.height * 2};
        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            shadowPass.addFrameBuffers(1,extent,{shadowMapView});
        }
    }

    void createForwardPass();

    void createImGuiPass();

    void createShadowPass();

    void createDescriptorSets(uint32_t count);

    void updateDescriptorSets(uint32_t count);

    void recordCommandBuffer(int idx);

    void recordCommandBuffers();

    void recordShadowPass(VkCommandBuffer commandBuffer,uint32_t des_idx);

    void recordForwardPass(VkCommandBuffer commandBuffer,uint32_t des_idx);

    void cleanupSwapChain();

    void destroyAllFrameBuffers(){
        forwardPass.destroyAllFrameBuffers();
        imGuiPass.destroyAllFrameBuffers();
        shadowPass.destroyAllFrameBuffers();
    }

public:
    VulkanResourceManager resourceManager;

    //global pipelines
    VulkanPipeline shadowPipeline;

    VulkanCommandPool commandPool;
    std::array<VulkanCommandPool,MAX_FRAME_IN_FLIGHT>  gui_commandPools{};

    std::vector<VkCommandBuffer> commandBuffers{};
    std::vector<VkCommandBuffer> gui_commandBuffers{};

    VulkanRenderPass forwardPass;
    VulkanRenderPass imGuiPass;
    VulkanRenderPass shadowPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout shadowDescriptorSetLayout;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;

    VulkanDescriptorPool descriptorPool;
    VulkanDescriptorPool imguiDescriptorPool;

    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDescriptorSet> shadowDescriptorSets;

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    MeshRender meshRender;

    std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>(Mesh::QuadMesh(10.0));
    MeshRender quadRender;

    void recreateSwapChain() override;
};
