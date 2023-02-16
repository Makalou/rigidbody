//
// Created by 王泽远 on 2023/2/16.
//

#ifndef RIGIDBODY_SHADERBASE_H
#define RIGIDBODY_SHADERBASE_H

struct ShaderInputLayout{

};

struct ShaderOutputLayout{

};

struct VertexShaderBase{
    virtual void main() = 0;
    ShaderInputLayout in;
    ShaderOutputLayout out;
};

struct FragmentShaderBase{
    virtual void main() = 0;
};

#define DEF_VERTEX_SHADER_BEGIN(name) struct name : VertexShaderBase {
#define DEF_VERTEX_SHADER_END(name) };

#define DEF_FRAGMENT_SHADER_BEGIN(name) struct name : FragmentShaderBase {
#define DEF_FRAGMENT_SHADER_END(name) };

#endif //RIGIDBODY_SHADERBASE_H
