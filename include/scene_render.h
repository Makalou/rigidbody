//
// Created by 王泽远 on 2023/2/1.
//

#ifndef RIGIDBODY_SCENE_RENDER_H
#define RIGIDBODY_SCENE_RENDER_H

#include "mesh.h"
#include <numeric>

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

//mesh id, matType id, matParam id, per instance data id
using MeshRenderingRecord = std::tuple<int,int,int,int>;

struct MeshRenderingPipelineContext{

    void bindPipeline(VkCommandBuffer cmd){
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipeline);
    }

    bool use_bindless() const {
        return false;
    }

    bool use_instancing() const {
        return false;
    }

    void bind_shader_param(VkCommandBuffer cmd,const MaterialParam& param){
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,layout,2,1, nullptr,0,0);
    }

    void bind_shader_param_array(VkCommandBuffer cmd,const std::vector<MaterialParam>& params){

    }

    void bind_instance_param(VkCommandBuffer cmd){
        vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,layout,3,1, nullptr,0,0);
    }

    void bind_instance_param_array(VkCommandBuffer cmd){

    }

    void record_draw_command(VkCommandBuffer cmd,bool bind_less, bool instancing){
        bindPipeline(cmd);
        if(bind_less){

            std::vector<int*> matID_views;
            for(auto & record : renderingList){
                matID_views.push_back(&std::get<2>(record));
            }
            std::sort(matID_views.begin(),matID_views.end(),[](const int* a, const int* b){return *a < *b;});
            std::vector<MaterialParam> shaderParams(matID_views.size());
            int current_range = -1;
            for(const auto &v : matID_views){
                if(*v != current_range){
                    current_range = *v;
                    shaderParams.push_back(matParams[current_range]);
                }
            }
            bind_shader_param_array(cmd,shaderParams);
            if(instancing){
                //Mesh sorting for instancing...
                std::sort(renderingList.begin(),renderingList.end(),
                          [](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
                {return std::get<0>(r1)<std::get<0>(r2);});
                auto mesh_batches =  get_mesh_batches(renderingList.begin(),renderingList.end());
                std::for_each(mesh_batches.begin(),mesh_batches.end(),[this,&cmd](batche_type & mesh_batch){
                    auto instance_count = std::distance(mesh_batch.first,mesh_batch.second);
                    //todo : gathering per instance data array [{transform, mat_id}, ... {transform, mat_id}]
                    //todo : bind array descriptor
                    bind_instance_param_array(cmd);
                    meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                });
            } else{
                std::for_each(renderingList.begin(),renderingList.end(),[this,cmd](MeshRenderingRecord & recorder){
                    //todo : bind per instance data {transform, mat_id}
                    bind_instance_param(cmd);
                    meshes[std::get<0>(recorder)].draw(cmd,1);
                });
            }
        } else{
            //Shader param batching...
            std::sort(renderingList.begin(),renderingList.end(),
                      [](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
            {return std::get<2>(r1)<std::get<2>(r2);});
            auto shader_param_batches = get_shader_param_batches(renderingList.begin(),renderingList.end());
            std::for_each(shader_param_batches.begin(),shader_param_batches.end(),
                          [this,instancing,&cmd](batche_type & shader_param_batch){

                auto shader_param = matParams[std::get<2>(*shader_param_batch.first)];
                bind_shader_param(cmd,shader_param);
                if(instancing){
                    //Mesh sorting for instancing...
                    std::sort(shader_param_batch.first,shader_param_batch.second,[](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
                    {return std::get<0>(r1)<std::get<0>(r2);});
                    auto mesh_batches= get_mesh_batches(shader_param_batch.first,shader_param_batch.second);
                    std::for_each(mesh_batches.begin(),mesh_batches.end(),[this,&cmd](batche_type & mesh_batch){
                        auto instance_count = std::distance(mesh_batch.first,mesh_batch.second);
                        //todo : gathering per instance data array [{transform}, ... {transform}]
                        //todo : bind array descriptors
                        meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                    });
                }else{
                    std::for_each(shader_param_batch.first,shader_param_batch.second,[this,cmd](MeshRenderingRecord & recorder){
                        //todo : bind per instance data {transform}
                        meshes[std::get<0>(recorder)].draw(cmd,1);
                    });
                }
            });
        }
    }

    std::vector<MeshRenderingRecord> renderingList;
    std::vector<MaterialParam> matParams;
    std::vector<Mesh> meshes;

    using list_it_type = decltype(renderingList.begin());
    using batche_type = std::pair<list_it_type,list_it_type>;

    std::vector<batche_type> get_mesh_batches(list_it_type beg, list_it_type end){
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
    }

    std::vector<batche_type> get_shader_param_batches(list_it_type beg, list_it_type end){
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
    }

    VkPipeline m_pipeline;
    VkPipelineLayout layout;
};

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
            std::sort(beg,end,[](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
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
        //Pipeline sorting...
        auto pipeline_batches = pipeline_batcher(renderingList.begin(),renderingList.end());
        std::for_each(pipeline_batches.begin(),pipeline_batches.end(),[this,&cmd](batche_type & pipeline_batch){
            //Bind Pipeline...
            auto mateiralType = matTypes[std::get<1>(*pipeline_batch.first)];
            mateiralType.bindPipeline(cmd);

            bool bind_less = use_bindless && mateiralType.use_bindless();
            bool instancing = use_instancing && mateiralType.use_instancing();

            if(bind_less){
                //todo : gathering shader param array
                std::vector<int> matIDs;
                std::for_each(pipeline_batch.first,pipeline_batch.second,[this, &matIDs](MeshRenderingRecord & record){
                    matIDs.emplace_back(std::get<2>(record));
                });
                std::vector<int*> matID_views(matIDs.size());
                std::transform(matIDs.cbegin(),matIDs.cend(),matID_views.begin(),[](int id){return &id;});
                std::sort(matID_views.begin(),matID_views.end(),[](int* a, int* b){return *a < *b;});

                std::vector<MaterialParam> shaderParams(matID_views.size());

                int cur_value = -1;
                int current_range = -1;
                for(const auto &v : matID_views){
                    if(*v == current_range){
                        *v = cur_value;
                    }else{
                        current_range = *v;
                        *v = cur_value++;
                        shaderParams.push_back(matParams[current_range]);
                    }
                }
                //todo : bind shader param array descriptor set

                if(instancing){
                    //Mesh sorting for instancing...
                    auto mesh_batcher = [](list_it_type beg,list_it_type end){
                        std::sort(beg,end,[](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
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
                        //todo : gathering per instance data array [{transform, mat_id}, ... {transform, mat_id}]
                        //todo : bind array descriptor
                        meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                    });
                } else{
                    std::for_each(pipeline_batch.first,pipeline_batch.second,[this,cmd,&matIDs](MeshRenderingRecord & recorder){
                        //todo : bind per instance data {transform, mat_id}
                        meshes[std::get<0>(recorder)].draw(cmd,1);
                    });
                }
            } else{
                //Shader param batching...
                auto shader_param_batcher = [](list_it_type beg,list_it_type end){
                    std::sort(beg,end,[](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
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
                        //Mesh sorting for instancing...
                        auto mesh_batcher = [](list_it_type beg,list_it_type end){
                            std::sort(beg,end,[](const MeshRenderingRecord& r1, const MeshRenderingRecord& r2)
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
                            //todo : gathering per instance data array [{transform}, ... {transform}]
                            //todo : bind array descriptors
                            meshes[std::get<0>(*mesh_batch.first)].draw(cmd,instance_count);
                        });
                    }else{
                        std::for_each(shader_param_batch.first,shader_param_batch.second,[this,cmd](MeshRenderingRecord & recorder){
                            //todo : bind per instance data {transform}
                            meshes[std::get<0>(recorder)].draw(cmd,1);
                        });
                    }
                });
            }
        });
    }

    void record_draw_command(VkCommandBuffer cmd){
        std::for_each(pipelineContexts.begin(),pipelineContexts.end(),[this, &cmd](MeshRenderingPipelineContext& pipeline){
            bool bind_less = use_bindless && pipeline.use_bindless();
            bool instancing = use_instancing && pipeline.use_instancing();
            pipeline.record_draw_command(cmd,bind_less,instancing);
        });
    }

    std::vector<MeshRenderingRecord> renderingList;
    std::vector<MaterialParam> matParams;
    std::vector<MaterialType> matTypes;
    std::vector<Mesh> meshes;
    std::vector<MeshRenderingPipelineContext> pipelineContexts;

    bool use_bindless;
    bool use_instancing;
};

#endif //RIGIDBODY_SCENE_RENDER_H
