//
// Created by 王泽远 on 2023/2/1.
//

#ifndef RIGIDBODY_SCENE_H
#define RIGIDBODY_SCENE_H

#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace scene {

    struct SceneGraph;

    struct Component{

    };

    struct MeshRenderer : Component{

    };

    struct Light : Component {

    };

    struct Viewer : Component{

    };

    struct SceneNode{

        SceneNode() = default;

        explicit SceneNode(SceneNode* _parent,std::string name,glm::vec3 translation = {.0,.0,.0},
                  glm::vec3 rotation = {.0,.0,.0},
                  glm::vec3 scale = {1.0,1.0,1.0}):parent(_parent),m_name(std::move(name)){
                    m_translation_to_parent = translation,
                    m_rotation_to_parent = rotation,
                    m_scale_to_parent = scale;
        }

        auto addChild(const std::string& child_name,
                           glm::vec3 translation = {.0,.0,.0},
                           glm::vec3 rotation = {.0,.0,.0},
                           glm::vec3 scale = {1.0,1.0,1.0}){
            //todo disallow same node name in the same level
            return children.emplace_back(std::make_shared<SceneNode>(this,child_name,translation,rotation,scale));
        }

        void removeNode(const std::string& path){
            auto t = path.find_first_of('/');
            auto rc_name = path.substr(0,t);
            for(auto it = children.begin();it!=children.end();++it){
                if((*it)->m_name == rc_name){
                    if(t == std::string::npos){
                        children.erase(it);
                        return;
                    }
                    return (*it)->removeNode(path.substr(t+1));
                }
            }
        }

        std::shared_ptr<SceneNode> getNode(const std::string& path) const{
            auto t = path.find_first_of('/');
            auto child_name = path.substr(0,t);
            for(auto child: children){
                if(child->m_name == child_name){
                    if(t == std::string::npos){
                        return child;
                    }
                    return child->getNode(path.substr(t+1));
                }
            }
            return nullptr;
        }

        void addComponent(std::unique_ptr<Component>&& component){
            components.emplace_back(std::move(component));
        }

    private:
        friend SceneGraph;
        std::string m_name;
        glm::vec3 m_translation_to_parent{};
        glm::vec3 m_rotation_to_parent {};
        glm::vec3 m_scale_to_parent{};
        std::vector<std::shared_ptr<SceneNode>> children;
        SceneNode* parent;
        std::vector<std::unique_ptr<Component>> components;
    };

    struct SceneDataTableCPU;

    struct SceneGraph {

        SceneGraph(){
            root_node = std::make_shared<SceneNode>(nullptr,"root");
        }

        void update(float deltaTime) {

        }

        auto addNodeToRoot(const std::string& name,
                          glm::vec3 translation = {.0,.0,.0},
                          glm::vec3 rotation = {.0,.0,.0},
                          glm::vec3 scale = {1.0,1.0,1.0}){
            return root_node->addChild(name,translation,rotation,scale);
        }

        void removeNode(const std::string& path){
            root_node->removeNode(path);
        }

        auto getNode(const std::string& path) const{
            return root_node->getNode(path);
        }
    private:
        std::shared_ptr<SceneNode> root_node;

        std::unique_ptr<SceneDataTableCPU> sceneDataTableCpu;
    };

    struct SceneDataTableCPU {
        std::vector<int> global_view_independent_params; // time, lights info etc.

        std::vector<int> pipelines;

        std::vector<int> materials;

        std::vector<int> textures;
        std::vector<int> shaders;
        std::vector<int> geometries; // meshes, points(particle) etc.

        std::vector<int> per_view_params; // camera info, skybox(clear value) info etc.
        std::vector<int> per_material_params; // shader param, textures etc.
        std::vector<int> per_obj_params; // model matrix etc. todo How about GPU skinning data?
    };

    struct SceneDataTableGPU {

        //pipelines
        std::vector<VkPipeline> graphics_pipelines;
        std::vector<VkPipeline> compute_pipelines;
        std::vector<VkPipeline> raytracing_pipelines;

        //scene global data (low binding frequency)
        VkBuffer scene_time;
        std::vector<VkBuffer> light_buffers;

        //material data (medium binding frequency)
        std::vector<VkBuffer> shaderParams;
        std::vector<VkImageView> textures;

        //draw data (high binding frequency)
        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkBuffer> indexBuffers;
        std::vector<VkBuffer> instanceBuffers;

        std::vector<VkDescriptorSet> descriptorSets;
    };
}
#endif //RIGIDBODY_SCENE_H
