#version 460
#extension GL_ARB_separate_shader_objects: enable

layout(location=0) centroid in vec2 TexCoord;
layout(location=1) in vec3 fragPos;
layout(location=2) smooth in vec3 fragNormal;
layout(location=3) in vec3 viewPos;
layout(location =4) in vec4 light_space_pos;

layout(location=0) out vec4 outColor;

layout(binding=1) uniform sampler2D texSampler;
layout(binding=2) uniform sampler2D shadowMapSampler;

void main()
{
    if(texture(texSampler,TexCoord).a==0){
        fragCoord.z = -1;
    }
}


