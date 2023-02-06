//
// Created by 王泽远 on 2023/2/6.
//

#include <stdexcept>

#include "system/window_system.h"
#include "win_config.h"
#include <iostream>
#include "camera.h"

namespace subsystem{

    namespace {
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
            //auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
            //app->setFrameBufferResized(true);
            std::cout<<"window resize: ["<<width<<","<<height<<"]\n";
        }

        static Camera *pCamera = new Camera();

        float deltaTime = 0.0f;

        static void cursorPosCallback(GLFWwindow *window, double x, double y) {
            static double old_x = -1;
            static double old_y = -1;
            if (old_x == -1 || old_y == -1) {
                old_x = x;
                old_y = y;
                return;
            }
            auto x_offset = x - old_x;
            old_x = x;
            auto y_offset = old_y - y;
            old_y = y;
            pCamera->deflect(x_offset, y_offset);
        }

        auto currentMouseVisible = GLFW_CURSOR_NORMAL;

        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GL_TRUE);
                    break;
                case GLFW_KEY_W:
                    pCamera->transfrom(CameraMovement::FORWARD, deltaTime);
                    break;
                case GLFW_KEY_S:
                    pCamera->transfrom(CameraMovement::BACKWARD, deltaTime);
                    break;
                case GLFW_KEY_A:
                    pCamera->transfrom(CameraMovement::LEFT, deltaTime);
                    break;
                case GLFW_KEY_D:
                    pCamera->transfrom(CameraMovement::RIGHT, deltaTime);
                    break;
                case GLFW_KEY_H:
                    if(action == GLFW_PRESS){
                        if (currentMouseVisible == GLFW_CURSOR_NORMAL) {
                            currentMouseVisible = GLFW_CURSOR_DISABLED;
                            glfwSetCursorPosCallback(window, cursorPosCallback);
                        } else {
                            currentMouseVisible = GLFW_CURSOR_NORMAL;
                            glfwSetCursorPosCallback(window, nullptr);
                        }
                        glfwSetInputMode(window, GLFW_CURSOR, currentMouseVisible);
                    }
                    break;

                default:
                    break;
            }
        }

        static void scrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
            pCamera->zoom(y_offset);
        }

        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

            }
        }
    }

    void WindowSystem::init() {
        if (glfwInit() == GLFW_FALSE) {
            throw std::runtime_error("glfwinit failed!");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIN_WIDTH, WIN_HIGHT, WIN_NAME, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        Camera::registry("camera1", pCamera);
    }

    bool WindowSystem::window_should_close() {
        return glfwWindowShouldClose(window);
    }

    void WindowSystem::cleanup() {
        glfwDestroyWindow(window);
    }
}

