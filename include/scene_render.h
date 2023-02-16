//
// Created by 王泽远 on 2023/2/1.
//

#ifndef RIGIDBODY_SCENE_RENDER_H
#define RIGIDBODY_SCENE_RENDER_H

#include "mesh.h"

struct SceneRenderer{

};

struct MaterialParam{
    void bindDescriptorSet(VkCommandBuffer cmd){

    }
};

struct MaterialType{
    void bindPipeline(VkCommandBuffer cmd){

    }

    bool use_bindless() const {
        return false;
    }

    bool use_instancing() const {
        return false;
    }
};

//mesh id, matType id, matParam id
using MeshRenderingRecorder = std::tuple<int,int,int,int>;

struct SceneMeshesRenderer : public SceneRenderer{

};

struct SceneStaticMeshesRenderer : public SceneMeshesRenderer{

    void frustum_culling_cpu(int cam_info){
        for(const auto & r : renderingList){
            auto mesh = meshes[std::get<0>(r)];
            auto local_bounding_sphere = mesh.get_local_bounding_sphere();
        }
    }

    void record_draw_rasterization_command(VkCommandBuffer cmd){
        using list_it_type = decltype(renderingList.begin());
        using batche_type = std::pair<list_it_type,list_it_type>;
        auto pipeline_batcher = [](list_it_type beg,list_it_type end){
            std::sort(beg,end,[](const MeshRenderingRecorder& r1,const MeshRenderingRecorder& r2)
                      {return std::get<1>(r1)<std::get<1>(r2);});
            std::vector<batche_type> batches;
            auto batch_begin = beg;
            auto batch_end = batch_begin;
            while (batch_end!=end){
                if(std::get<1>(*batch_end)!=std::get<1>(*batch_begin)){
                    batches.emplace_back(batch_begin,batch_end);
                    batch_begin = batch_end;
                }
                batch_end++;
            }
            batches.emplace_back(batch_begin,batch_end);
            return batches;
        };
        //pipeline sorting
        auto pipeline_batches = pipeline_batcher(renderingList.begin(),renderingList.end());
        std::for_each(pipeline_batches.begin(),pipeline_batches.end(),[this,&cmd](batche_type & pipeline_batch){
            //Bind Pipeline
            auto mateiralType = matTypes[std::get<1>(*pipeline_batch.first)];
            mateiralType.bindPipeline(cmd);

            bool bind_less = use_bindless && mateiralType.use_bindless();
            bool instancing = use_instancing && mateiralType.use_instancing();

            if(bind_less){
                //todo : gathering shader param array
                std::for_each(pipeline_batch.first,pipeline_batch.second,[this](MeshRenderingRecorder & recorder){
                    std::vector<MaterialParam> shader_params;
                    shader_params.emplace_back(matParams[std::get<2>(recorder)]);
                });
                //todo : bind shader param array descriptor set
                if(instancing){
                    //Mesh sorting for instancing
                    auto mesh_batcher = [](list_it_type beg,list_it_type end){
                        std::sort(beg,end,[](const MeshRenderingRecorder& r1,const MeshRenderingRecorder& r2)
                        {return std::get<0>(r1)<std::get<0>(r2);});
                        std::vector<batche_type> batches;
                        auto batch_begin = beg;
                        auto batch_end = batch_begin;
                        while (batch_end!=end){
                            if(std::get<0>(*batch_end)!=std::get<0>(*batch_begin)){
                                batches.emplace_back(batch_begin,batch_end);
                                batch_begin = batch_end;
                            }
                            batch_end++;
                        }
                        batches.emplace_back(batch_begin,batch_end);
                        return batches;
                    };
                    auto mesh_batches =  mesh_batcher(pipeline_batch.first,pipeline_batch.second);
                    std::for_each(mesh_batches.begin(),mesh_batches.end(),[this,&cmd](batche_type & mesh_batch){
                        auto instance_count = std::distance(mesh_batch.first,mesh_batch.second);
                        //todo : gathering per instance data array [{transform, mat_id}]
                        //todo : bind array descriptors
                        meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                    });
                } else{
                    std::for_each(pipeline_batch.first,pipeline_batch.second,[this,cmd](MeshRenderingRecorder & recorder){
                        //todo : bind per instance data {transform, mat_id}
                        meshes[std::get<0>(recorder)].draw(cmd,1);
                    });
                }
            } else{
                //Shader param batching
                auto shader_param_batcher = [](list_it_type beg,list_it_type end){
                    std::sort(beg,end,[](const MeshRenderingRecorder& r1,const MeshRenderingRecorder& r2)
                    {return std::get<2>(r1)<std::get<2>(r2);});
                    std::vector<batche_type> batches;
                    auto batch_begin = beg;
                    auto batch_end = batch_begin;
                    while (batch_end!=end){
                        if(std::get<0>(*batch_end)!=std::get<2>(*batch_begin)){
                            batches.emplace_back(batch_begin,batch_end);
                            batch_begin = batch_end;
                        }
                        batch_end++;
                    }
                    batches.emplace_back(batch_begin,batch_end);
                    return batches;
                };
                auto shader_param_batches = shader_param_batcher(pipeline_batch.first,pipeline_batch.second);
                std::for_each(shader_param_batches.begin(),shader_param_batches.end(),[this,instancing,&cmd](batche_type & shader_param_batch){
                    auto shader_param = matParams[std::get<2>(*shader_param_batch.first)];
                    shader_param.bindDescriptorSet(cmd);
                    if(instancing){
                        //Mesh sorting for instancing
                        auto mesh_batcher = [](list_it_type beg,list_it_type end){
                            std::sort(beg,end,[](const MeshRenderingRecorder& r1,const MeshRenderingRecorder& r2)
                            {return std::get<0>(r1)<std::get<0>(r2);});
                            std::vector<batche_type> batches;
                            auto batch_begin = beg;
                            auto batch_end = batch_begin;
                            while (batch_end!=end){
                                if(std::get<0>(*batch_end)!=std::get<0>(*batch_begin)){
                                    batches.emplace_back(batch_begin,batch_end);
                                    batch_begin = batch_end;
                                }
                                batch_end++;
                            }
                            batches.emplace_back(batch_begin,batch_end);
                            return batches;
                        };
                        auto mesh_batches =  mesh_batcher(shader_param_batch.first,shader_param_batch.second);
                        std::for_each(mesh_batches.begin(),mesh_batches.end(),[this,&cmd](batche_type & mesh_batch){
                            auto instance_count = std::distance(mesh_batch.first,mesh_batch.second);
                            //todo : gathering per instance data array [{transform}]
                            //todo : bind array descriptors
                            meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                        });
                    }else{
                        std::for_each(shader_param_batch.first,shader_param_batch.second,[this,cmd](MeshRenderingRecorder & recorder){
                            //todo : bind per instance data {transform}
                            meshes[std::get<0>(recorder)].draw(cmd,1);
                        });
                    }
                });
            }
        });
    }

    std::vector<MeshRenderingRecorder> renderingList;
    std::vector<MaterialParam> matParams;
    std::vector<MaterialType> matTypes;
    std::vector<Mesh> meshes;

    bool use_bindless;
    bool use_instancing;
};

#endif //RIGIDBODY_SCENE_RENDER_H
