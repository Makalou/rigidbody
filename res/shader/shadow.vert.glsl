#version 460
#extension GL_GOOGLE_include_directive: enable
layout(location=0) in vec3 inPosition;

#include "built_in.glsl"

void main(){
    gl_Position = built_in.light_sapce_matrix * built_in.model*vec4(inPosition,1.0);
}
