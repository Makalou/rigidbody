//
// Created by 王泽远 on 2023/2/16.
//

#ifndef RIGIDBODY_MYSHADER_H
#define RIGIDBODY_MYSHADER_H

#include "shaderBase.h"


DEF_VERTEX_SHADER_BEGIN(MyVertexShader)

    float f1(){

    }

    float f2(){
        return f1();
    }

    void main() override{

    }

DEF_VERTEX_SHADER_END(MyVertexShader)

DEF_FRAGMENT_SHADER_BEGIN(MyFragmentShader)

    float f1(){

    }

    float f2(){
        return f1();
    }

    void main() override{

    }

DEF_FRAGMENT_SHADER_END(MyFragmentShader)

#endif //RIGIDBODY_MYSHADER_H
