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

