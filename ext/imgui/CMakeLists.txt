cmake_minimum_required(VERSION 3.20)
# Change template to your own project name
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(imgui)

include_directories(../glfw/include)
include_directories(../imgui/include)

add_library(imgui STATIC
        src/imgui_widgets.cpp
        src/imgui_tables.cpp
        src/imgui_impl_vulkan.cpp
        src/imgui_impl_glfw.cpp
        src/imgui_draw.cpp
        src/imgui.cpp
        src/imgui_demo.cpp
        src/implot.cpp
        src/implot_items.cpp)

find_package(Vulkan REQUIRED FATAL_ERROR)
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    #set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    #STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    message(STATUS "Successfully located the Vulkan SDK at: ${Vulkan_INCLUDE_DIRS}")
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()


