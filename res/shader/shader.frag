#version 460
#extension GL_ARB_separate_shader_objects: enable
#extension GL_GOOGLE_include_directive: enable

layout(location = 0) centroid in vec2 TexCoord;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 light_space_pos;

layout(location = 0) out vec4 outColor;

#include "built-in/built_in.glsl"

layout(set = CUSTOM_SET_BASE,binding = 0) uniform sampler2D albedoSampler;
layout(set = CUSTOM_SET_BASE,binding = 1) uniform sampler2D emissiveSampler;
layout(set = CUSTOM_SET_BASE,binding = 2) uniform sampler2D normalSampler;
layout(set = CUSTOM_SET_BASE,binding = 3) uniform sampler2D metalSampler;
layout(set = CUSTOM_SET_BASE,binding = 4) uniform sampler2D roughSampler;
layout(set = CUSTOM_SET_BASE,binding = 5) uniform sampler2D brdfLUT;

#include "built-in/utility/transform_functions.glsl"
#include "built-in/utility/rng.glsl"
#include "built-in/utility/random_sample.glsl"
#include "built-in/utility/color_space.glsl"
#include "built-in/shading/pbr.glsl"
#include "built-in/post-processing/tonemapping.glsl"

vec3 lightColor={4.0,4.0,4.0};
vec3 lightDir={-1.0,1.0,1.0};

const float constant=1.0;
const float linear=0.09;
const float quadratic=0.032;

const float near=0.01;
const float far=100.0;

const vec3 caches[8] = {vec3(0,0,0),vec3(1,0,0),vec3(0,1,0),
						vec3(0,0,1),vec3(1,1,0),vec3(1,0,1),
						vec3(0,1,1),vec3(1,1,1)};

void main()
{

	float shadow = 0;

	vec3 norm=normalize(fragNormal);
	mat3 TBN = computeTBN(norm,fragPos,TexCoord);
	vec3 N = normalize(TBN*(2.0*vec3(texture(normalSampler,TexCoord))-1.0));
	vec3 V = normalize(cam.pos-fragPos);
	float NdotV = max(dot(N,V),0.0);

	vec3 light_space_ndc = light_space_pos.xyz/light_space_pos.w;

	vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

	vec2 texelSize = 1.0 / textureSize(shadowMapSampler, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMapSampler, shadow_map_coord.xy + vec2(x, y) * texelSize).r;
			shadow += light_space_ndc.z > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	shadow = 1.0-shadow;

	vec3 albedo = texture(albedoSampler,TexCoord).rgb;
	inverse_gama_correct(albedo);
	float metallic = texture(metalSampler,TexCoord).r;
	float roughness = texture(roughSampler,TexCoord).r;
	vec3 emissive = vec3(texture(emissiveSampler,TexCoord));

	vec3 F0 = vec3(0.04);
	F0 = mix(F0,albedo,metallic);

	//reflectance equation
	vec3 Lo = vec3(0.0);

	for(int i=0;i<1;++i){
		vec3 L = normalize(lightDir);

		float NdotL = max(dot(N, L), 0.0);

		float attenuation = 1.0;

		vec3 Li = lightColor*attenuation*shadow;

		Lo += brdf(L,V,N,albedo,metallic,roughness,F0) * Li * NdotL;
	}

	// ambient lighting (we now use IBL as the ambient term)
	vec3 F = fresnelSchlickRoughness(NdotV, F0,roughness);
	vec2 envBRDF  = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = vec3(0.01) * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = getIBLAbiment(N,V,F0,metallic,roughness,vec3(0.01),albedo)+specular;

	//Screen Space Ambient Occlusion
	float ao_factor = 0.0;

	vec2 p = gl_FragCoord.xy;
	float seed = float(baseHash(floatBitsToUint(p)))/float(0xffffffffU);

	for(int i = 0;i<50;++i){
		//choose a random point inside hemisphere
		vec3 sample_point = sample_inside_hemisphere(fragPos,N,seed);
		//mapping the sample point to screen space and test if the sample point visible from camera
		vec4 sample_ndc = cam.proj * cam.view * vec4(sample_point,1.0);
		float sample_depth = 0.5*sample_ndc.z-0.5;
		if(gl_FragCoord.z>sample_depth) ao_factor+=1.0;
		//refresh seed
		seed = hash1(seed);
	}

	ao_factor /= 50.0;

	vec3 color = (ao_factor)*ambient + Lo + emissive;

	//tone mapping
	color = sigmod(color);
	//gama correct
	gama_correct(color);

	float mml = textureQueryLod(albedoSampler, TexCoord).x;
	/*
	if(specular.x < 0.001)
		if(length(fract(pow(2,-mml)*100*TexCoord)-vec2(0.5))<=0.1)
			color = vec3(1.0,0.0,0.0);
		else
			color = vec3(0.0,0.0,0.0);
	else
		color = vec3(1.0,0.0,0.0);
		*/
	vec4 som = cam.proj * cam.view * vec4(fragPos,1.0);
	vec3 ndc = (som.xyz/som.w)*0.5+vec3(0.5);
	int level = int(floor(7.0*linearize_depth(near,far,ndc.z)));
	const int map_len = int(pow(2,level));
	int map_size = map_len*map_len;
	ivec2 hh = ivec2(floor(map_len*ndc.xy));
	int index = int(fma(map_len-1,hh.x,hh.y));
	//ivec3 hh = ivec3(floor(map_len*(ndc)));
	//ivec3 nh = ivec3(sign(fragNormal)+vec3(1.0));
	//int normal_idx = int(fma(2,fma(2,nh.x,nh.y),nh.z));
	//int index = int(fma(map_len-1,fma(map_len-1,hh.x,hh.y),hh.z)+26*normal_idx);
	//map_size*=27;
	//color = caches[(index)%8];//vec3(float(index)/float(map_size));
	outColor=vec4(color,1.0);
}

