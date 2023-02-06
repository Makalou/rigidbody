//
// Created by 王泽远 on 2023/2/6.
//

#ifndef RIGIDBODY_WINDOW_SYSTEM_H
#define RIGIDBODY_WINDOW_SYSTEM_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace subsystem{
    class WindowSystem{
    public:
        void init();

        bool window_should_close();

        void cleanup();

        GLFWwindow* get_window() const{
            return window;
        }
    private:
        GLFWwindow* window;
    };
}

#endif //RIGIDBODY_WINDOW_SYSTEM_H
