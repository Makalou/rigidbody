//
// Created by 王泽远 on 2023/2/6.
//

#ifndef RIGIDBODY_RENDER_SYSTEM_H
#define RIGIDBODY_RENDER_SYSTEM_H

#include "vk_backend.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include "scene_render.h"
#include "render_graph.h"

namespace subsystem{
    class RenderSystem{
    public:
        void init(MyBackend* backend){
            gpu_backend = backend;
        }

        void init_resource(){
            
        }

        void renderGUI(VkCommandBuffer commandBuffer, uint32_t index);

        void drawFrame(int currentFrame,bool* frameBufferResized,bool rota);
    private:
        MyBackend* gpu_backend;
        std::unique_ptr<SceneRenderer> sceneRenderer;
        std::vector<RenderGraph> renderGraphs;
    };
}
#endif //RIGIDBODY_RENDER_SYSTEM_H
