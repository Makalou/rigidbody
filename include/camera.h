#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <iostream>
#include "glm/ext/matrix_clip_space.hpp"

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -100.0f;
const float PITCH = 0.0f;
const float SPEED = 500.0f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 60.0f;

struct CameraInfo{
    alignas(16)glm::vec3 pos;
    alignas(16)glm::vec3 up;
    alignas(16)glm::vec3 right;
    alignas(16)glm::vec3 front;
    alignas(16)glm::vec3 worldUp;
    //camera models
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Camera {
public:
	explicit Camera(glm::vec3 position = glm::vec3(2.0f, 2.0f, 7.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	glm::vec3 pos() const{
		return m_info.pos;
	}
	void transfrom(CameraMovement direction, float deltaTime);
	void deflect(float xoffset, float yoffset, bool constrainPitch = true);
	void zoom(float offset);
	glm::mat4 lookAt();
	float fov() const{
		return glm::radians(m_zoom);
	}
public:
	static Camera* getCamera(const std::string & name);
	static void registry(const std::string& name, Camera* camera);
	void updateCameraVectors();
	void debug() {
		std::cout << "{"<<m_info.pos.x <<","<<m_info.pos.y<<","<<m_info.pos.z<<"}" << std::endl;
		std::cout << "{"<<m_info.up.x <<","<<m_info.up.y<<","<<m_info.up.z<<"}" << std::endl;
		std::cout << "{"<<m_info.front.x <<","<<m_info.front.y<<","<<m_info.front.z<<"}" << std::endl;
	}
private:
    CameraInfo m_info;
public:

    CameraInfo get_camera_info(){
        m_info.view = lookAt();
        m_info.proj=glm::perspective(fov(), 16.f/9.f, 0.1f, 100.0f);
        m_info.proj[1][1] *= -1;
        return m_info;
    }
private:

	std::string name;
    //camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;
    float Yaw;
    float Pitch;
private:
	static std::unordered_map<std::string,Camera*> camera_instances;
};

