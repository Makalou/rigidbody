#pragma once
#define GLM_FORCE_INLINE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

#include <chrono>
#include <iostream>

#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

static float getDeltaTime()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}

struct UniformBufferObject {

	alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 lightSapceMatrix;
    alignas(16) float time;
	
	static void update(UniformBufferObject& ubo,bool rotate) {
		Camera* camera = Camera::getCamera("camera1");
		float time = rotate?getDeltaTime():0;
		auto m = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ubo.model = glm::rotate(m, time*glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.time = getDeltaTime();
        const glm::mat4 light_view = glm::lookAt(glm::vec3(-5,5,5),glm::vec3(0,0,0),glm::vec3(0,1,0));
        //ubo.view = light_view;
        ubo.lightSapceMatrix = camera->get_camera_info().proj * light_view;
	}
};
