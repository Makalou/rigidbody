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

namespace subsystem{
    class RenderSystem{
    public:
        void init(MyBackend* backend){
            gpu_backend = backend;
        }

        void renderGUI(VkCommandBuffer commandBuffer, uint32_t index);

        void drawFrame(int currentFrame,bool* frameBufferResized,bool rota);
    private:
        MyBackend* gpu_backend;
    };
}
#endif //RIGIDBODY_RENDER_SYSTEM_H
