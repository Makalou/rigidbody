#include "utility.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>

std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file:"+filename+" !");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void loadModelObj(Mesh& mesh, const std::string path)
{

	tinyobj::attrib_t attrib;//holders all of the positions, normals texture coordinates
	std::vector<tinyobj::shape_t>shapes;//contains all of the separate objects and their faces
	std::vector<tinyobj::material_t> materials;//
	std::string warn, err;
    auto mtl_basedir = path.substr(0,path.find_last_of("/"));
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(),mtl_basedir.c_str());
    if(!err.empty()){
        throw std::runtime_error(warn + err);
    }else if(!warn.empty()){
        std::cerr<<warn<<"n";
    }

	std::unordered_map<Vertex, uint32_t>uniqueVertices = {};

    std::vector<Vertex> vertices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};
			vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],attrib.vertices[3 * index.vertex_index + 1],attrib.vertices[3 * index.vertex_index + 2] };
            if(!attrib.texcoords.empty())
			    vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],1.0f-attrib.texcoords[2 * index.texcoord_index + 1] };
            if(!attrib.normals.empty())
			    vertex.normal = { attrib.normals[3 * index.normal_index + 0],attrib.normals[3 * index.normal_index + 1],attrib.normals[3*index.normal_index+2] };
			vertex.color = { 1.0f,1.0f,1.0f,1.0f};

			if (uniqueVertices.find(vertex)==uniqueVertices.cend()) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			mesh.indies.push_back(uniqueVertices[vertex]);
		}
	}

    mesh.load_vertices(vertices);
}

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

void loadModelGLTF(Mesh& mesh, const std::string& path,bool binary = false){

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret;

    if(binary){
        ret = loader.LoadBinaryFromFile(&model, &err, &warn,path);
    }else{
        ret = loader.LoadASCIIFromFile(&model, &err, &warn,path);
    }

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
    }

    for(const auto& scene : model.scenes){
        for(const int & root_idx : scene.nodes){
            auto current_node = model.nodes[root_idx];
            auto scale = current_node.scale;
            auto rotation = current_node.rotation;
            auto translation = current_node.translation;
            auto mesh = model.meshes[current_node.mesh];
            for(const auto & primitive : mesh.primitives){
                auto attributes = primitive.attributes;
                auto material = model.materials[primitive.material];
            }
        }
    }
}
