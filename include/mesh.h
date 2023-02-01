//
// Created by 王泽远 on 2022/7/6.
//

#ifndef RIGIDBODY_MESH_H
#define RIGIDBODY_MESH_H

#include "vertex.h"
#include "vk_device.h"

struct vertexPositionView{
public:
    void operator=(const glm::vec3 & p){
        for(auto & p_pos : p_positions)
            *p_pos = p;
    }
    glm::vec3& operator*(){
        return *p_positions[0];
    }
    void reg(glm::vec3* p){
        p_positions.push_back(p);
    }

    bool operator == (const glm::vec3& v){
        return v == *p_positions[0];
    }

    void operator +=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos += v;
    }

    void operator -=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos -= v;
    }

    const glm::vec3 * operator->() const {
        return p_positions[0];
    }

private:
    std::vector<glm::vec3*> p_positions;
};

struct Mesh {

    void load_vertices(const std::vector<Vertex>& _vertices){
        vertices=_vertices;
        update_position_views();
    }

    std::vector<Vertex>& getVertices(){
        return vertices;
    }

    auto get_position_view(){
        return position_views;
    }

    auto get_vertex_size() const{
        return sizeof (vertices[0]);
    }

    auto get_vertices_count() const{
        return vertices.size();
    };

    auto vertices_buffer_size() const{
        return sizeof (vertices[0])*vertices.size();
    }

    auto get_vertices_data() const{
        return vertices.data();
    }

    auto get_index_size() const{
        return sizeof (indies[0]);
    }

    auto get_indices_count() const{
        return indies.size();
    };

    auto indices_buffer_size() const{
        return sizeof (indies[0])*indies.size();
    }

    auto get_indices_data() const{
        return indies.data();
    }

    void create_device_obj(const VulkanDevice& device){
        VkDeviceSize bufferSize = vertices_buffer_size();
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_device_obj.m_vertexBuffer, m_device_obj.m_vertexMemory);

        bufferSize = indices_buffer_size();
        if(bufferSize==0) return;
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                            ,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            m_device_obj.m_indexBuffer,
                                    m_device_obj.m_indexMemory);
    }

    void upload_vertex_data_to_device(const VulkanDevice& device){
        VkDeviceSize bufferSize = vertices_buffer_size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    stagingBuffer,stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, get_vertices_data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        device.copyBuffer(stagingBuffer, m_device_obj.m_vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void upload_indices_data_to_device(const VulkanDevice& device){
        if(indies.empty()) return;
        VkDeviceSize bufferSize = indices_buffer_size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, get_indices_data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        device.copyBuffer(stagingBuffer, m_device_obj.m_indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void free_from_device(const VulkanDevice& device){
        vkDestroyBuffer(device, m_device_obj.m_vertexBuffer, nullptr);
        vkFreeMemory(device, m_device_obj.m_vertexMemory, nullptr);
        vkDestroyBuffer(device, m_device_obj.m_indexBuffer, nullptr);
        vkFreeMemory(device, m_device_obj.m_indexMemory, nullptr);
    }

    std::vector<uint32_t> indies;
    std::vector<Vertex> vertices = {};
    std::vector<vertexPositionView> position_views = {};

    struct DeviceObject{
        VkDeviceMemory m_vertexMemory;
        VkDeviceMemory m_indexMemory;
        VkBuffer m_vertexBuffer;
        VkBuffer m_indexBuffer;
    }m_device_obj;

    /*
    std::vector<uint32_t> indies = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
    };
     */

private:
    void update_position_views(){
        for(auto & v : vertices){
            bool find = false;
            for(auto & view : position_views){
                if(view == v.pos){
                    view.reg(&v.pos);
                    find = true;
                    break;
                }
            }
            if(!find){
                vertexPositionView view;
                view.reg(&v.pos);
                position_views.push_back(view);
            }
        }
    };

public:
    static Mesh QuadMesh(float edge_length){
        Mesh mesh;
        const float quad_half_length = edge_length/2;
        mesh.vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        mesh.vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        mesh.vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        mesh.vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        mesh.vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        mesh.vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
        return mesh;
    }
};

#endif //RIGIDBODY_MESH_H
