#version 120

layout(set=1,binding=0) uniform PBR_Uniform{
    float shininess;
    float ior;
    float dissolve;
    float illum;
    float dummy;
    float roughness;
    float metallic;
    float sheen;
    float clearcoat_thickness;
    float clearcoat_roughness;
    float anisotropy;
    float anisotropy_rotation;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
}pbr_uniform;

layout(set=1,binding=1) uniform sample2D ambientMap;
layout(set=1,binding=2) uniform sample2D specularMap;
layout(set=1,binding=3) uniform sample2D specularHeighightMap;
layout(set=1,binding=4) uniform sample2D alphaMap;
layout(set=1,binding=5) uniform sample2D reflectionMap;
layout(set=1,binding=6) uniform sample2D roughnessMap;
layout(set=1,binding=7) uniform sample2D metallicMap;
layout(set=1,binding=8) uniform sample2D sheenMap;
layout(set=1,binding=9) uniform sample2D emissiveMap;
layout(set=1,binding=10) uniform sample2D normalMap;

void main() {
    //do some compute
}
