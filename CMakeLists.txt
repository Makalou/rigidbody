cmake_minimum_required(VERSION 3.22)
project(rigidbody)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(ext ext_build)

include_directories(include)
include_directories(ext/glm)
include_directories(ext/glfw/include)
include_directories(ext/tinyobjloader)
include_directories(ext/imgui/include)
include_directories(ext/vma/include)

add_executable(rigidbody src/main.cpp src/Application.cpp src/system/window_system.cpp src/camera.cpp src/utility.cpp src/vk_backend.cpp src/vk_resource_manager.cpp include/vulkan_memory_utils.h include/vulkan_image_utils.h include/vk_graphics_pipeline.h include/vk_command_manager.h src/VkBootstrap.cpp src/spirv_reflect.c include/vk_descriptor.h src/my_backend.cpp include/vk_device.h  include/material.h include/texture.h src/mesh.cpp include/mesh.h include/mesh_render.h include/vk_renderpass.h include/scene_render.h src/system/physics_system.cpp src/mesh_render.cpp src/system/render_system.cpp include/vk_queue.h include/shader/shaderBase.h)

find_package(glfw3 REQUIRED)
target_link_libraries(rigidbody PRIVATE glfw)

find_package(Vulkan REQUIRED FATAL_ERROR)
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    #set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    #STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    message(STATUS "Successfully located the Vulkan SDK at: ${Vulkan_INCLUDE_DIRS}")
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

target_link_libraries (rigidbody PRIVATE Vulkan::Vulkan)

target_link_libraries(rigidbody PRIVATE imgui)

#target_link_libraries(rigidbody PRIVATE vma)
target_link_libraries(rigidbody PRIVATE tinygltf)