//
// Created by 王泽远 on 2023/2/6.
//

#include "system/physics_system.h"

#include "mesh.h"

namespace subsystem{
    void PhysicsSystem::init(const std::shared_ptr<Mesh>& _mesh) {
        mesh = _mesh;
        auto vertex_positions = _mesh->get_position_view();

        local_state.resize(vertex_positions.size());

        const float per_vert_mass = mass/local_state.size();
        gravity = mass*g;

        for(size_t i =0;i<local_state.size();++i)
            local_state[i] = *vertex_positions[i];

        for (auto & p: local_state) {
            center+=p;
        }

        center/=local_state.size();

        for (auto & pos: local_state) {
            pos-=center;
            collision_radius = fmax(collision_radius,glm::length(pos));
            //pos*=0.2;
        }

        //convex_hull(local_state);

        float w11 = 0,w12=0,w13=0,w22=0,w23=0,w33=0;

        for (auto & pos: local_state) {
            w11 += (pos.y*pos.y+pos.z*pos.z)*per_vert_mass;
            w12 -= pos.x*pos.y*per_vert_mass;
            w13 -= pos.x*pos.z*per_vert_mass;
            w22 += (pos.x*pos.x+pos.z*pos.z)*per_vert_mass;
            w23 -= pos.y*pos.z*per_vert_mass;
            w33 += (pos.x*pos.x+pos.y*pos.y)*per_vert_mass;
        }

        I0_inv = glm::inverse(glm::mat3{w11,w12,w13,w12,w22,w23,w13,w23,w33});

        R = glm::mat3_cast(q);
        for(size_t i =0;i<vertex_positions.size();++i){
            vertex_positions[i]=x+R*local_state[i];
        }
    }

    void PhysicsSystem::update(float deltaTime) {
        auto vertex_positions = mesh->get_position_view();
        auto dx = P / mass;
        R = glm::mat3_cast(q);
        auto I_inv = R * I0_inv * glm::transpose(R);
        glm::vec3 w = I_inv * L;
        //glm::mat3 w_star = {0,w.z,-w.y,-w.z,0,w.x,w.y,-w.x,0};
        //std::cout<<"["<<w.x<<","<<w.y<<","<<w.z<<"]";
        //std::cout<<glm::determinant(glm::mat3_cast(q))<<"\n";
        auto wq = glm::qua(0.f,w);
        auto dq = 0.5f*wq*q;
        //std::cout<<"dq : "<<"["<<dq.w<<","<<dq.x<<","<<dq.y<<","<<dq.z<<"]\n";
        //std::cout<<"q : "<<"["<<q.w<<","<<q.x<<","<<q.y<<","<<q.z<<"]\n";
        x += dx*deltaTime;
        q += dq*deltaTime;
        q = glm::normalize(q);
        R = glm::mat3_cast(q);
        P += gravity * deltaTime;
        P *= damping;L *= damping;
        for (size_t i = 0; i < vertex_positions.size(); ++i) {
            vertex_positions[i] = x + R * local_state[i];
        }
#define ENABLE_COLLISION
#ifdef ENABLE_COLLISION
        if(center.y-collision_radius<=0){
            glm::vec3 deltaP{};
            glm::vec3 deltaL{};
            collide_count = 0;
            for (size_t i = 0; i < vertex_positions.size(); ++i) {
                auto p = vertex_positions[i];
                if (p->y < 0.00f&&p->x<=2.0f&&p->x>=-2.0f&&p->z<=2.0f&&p->z>=-2.0f) {
                    auto r = R*local_state[i];
                    float v_normal = (P / mass + glm::cross(w, r)).y;
                    if (v_normal<=0) {
                        //std::cout<<v_normal<<"\n";
                        float j1 = -(1.f + 0.3f) * v_normal;
                        float j2 = 1 / mass + (I_inv * glm::cross(glm::cross(r, {0.f, 1.0f, 0.f}), r)).y;
                        float j = j1 / j2;
                        auto J = glm::vec3{0, j, 0};
                        deltaP += J;
                        deltaL += glm::cross(r, J);
                        collide_count++;
                    }
                }
            }
            if(collide_count>0) {
                deltaP/=collide_count;deltaL/=collide_count;
                //std::cout<<collide_count<<"\n";
            }
            P += deltaP;L += deltaL;

        }
#endif
    }
}
