//
// Created by 王泽远 on 2023/2/6.
//

#ifndef RIGIDBODY_PHYSICS_SYSTEM_H
#define RIGIDBODY_PHYSICS_SYSTEM_H

#include "glm/gtc/quaternion.hpp"
#include <vector>
#include "mesh.h"

namespace subsystem{
    class PhysicsSystem{
    public:
        void init(const std::shared_ptr<Mesh>& _mesh);

        void update(float deltaTime);
    private:
        const float scale = 1.0f;

        glm::vec3 x = {0.f,5.f,0.f};
        glm::qua<float> q{glm::radians(glm::vec3 (0.f,0.f,0.f))};
        glm::vec3 P = {};
        glm::vec3 L = {0,0,0};

        glm::mat<3,3,float> R;

        glm::mat3 I0_inv;

        const glm::vec3 g = {0.f, -9.8f, 0.f};
        glm::vec3 gravity;
        const float damping = 0.994f;

        std::vector<float> v_y;
        std::vector<float> energy;

        size_t vex_num;

        const float mass = 10.0f;

        bool collide = false;
        bool rota = false;

        int collide_count = 0;

        glm::vec3 center{};

        float collision_radius = 0;

        std::vector<glm::vec3> local_state;

        std::shared_ptr<Mesh> mesh;
    };
}
#endif //RIGIDBODY_PHYSICS_SYSTEM_H
