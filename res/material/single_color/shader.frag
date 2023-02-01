#version 460
#extension GL_ARB_separate_shader_objects: enable

#define PI 3.1415926535

vec3 CalcDirLight(vec3 normal, vec3 viewDir);
vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);

layout(location=0) centroid in vec2 TexCoord;
layout(location=1) in vec3 fragPos;
layout(location=2) smooth in vec3 fragNormal;
layout(location =3) in vec4 light_space_pos;

layout(location=0) out vec4 outColor;

layout(set = 0,binding=1) uniform CamInfo{
    vec3 pos;
    vec3 up;
    vec3 right;
    vec3 front;
    vec3 worldUp;
//camera models
    mat4 view;
    mat4 proj;
}cam;

layout(set =0,binding=2) uniform sampler2D shadowMapSampler;

vec3 lightColor={3.0,3.0,3.0};
vec3 lightDir={-1.0,1.0,1.0};

const float constant=1.0;
const float linear=0.09;
const float quadratic=0.032;

const float near=0.1;
const float far=100.0;

void main()
{
    vec3 objectColor=vec3(0.3,0.3,0.3);

    float shadow = 1;

    vec3 light_space_ndc = light_space_pos.xyz/light_space_pos.w;

    vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

    float closeDepth = texture(shadowMapSampler,shadow_map_coord).x;

    if(light_space_ndc.z > closeDepth)
    shadow = 0.0;
    lightDir=normalize(lightDir);
    //calculate ambient
    float ambientStrength=0.1;
    vec3 ambient = ambientStrength*lightColor*objectColor;
    //calculate diffuse
    vec3 norm=normalize(fragNormal);
    float diff=max(dot(norm,lightDir),0.0);
    vec3 diffuse=diff*lightColor*shadow*objectColor;
    //caculate specular
    vec3 viewDir=normalize(cam.pos-fragPos);
    vec3 halfDir=normalize(lightDir+viewDir);
    float alpha=max(dot(viewDir,halfDir),0.0);
    float spec=pow(alpha,32);
    vec3 specular=0.1*spec*lightColor*shadow*objectColor;

    vec3 result=(ambient+diffuse+specular);//*attenuation;

    outColor=vec4(result,1.0);
}

vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    return vec3(0.0,0.0,0.0);
}
vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    return vec3(0.0,0.0,0.0);
}


