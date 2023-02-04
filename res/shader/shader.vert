#version 460
#extension GL_GOOGLE_include_directive: enable

layout(location=0) in vec3 inPosition;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inTexCoord;
layout(location=3) in vec3 inNormal;

layout(location=0) out vec2 fragTexCoord;
layout(location=1) out vec3 fragPos;
layout(location=2) out vec3 fragNormal;
layout(location=3) out vec4 light_sapce_pos;

#include "built_in.glsl"

void main(){
	vec4 world_pos = built_in.model*vec4(inPosition,1.0f);
	light_sapce_pos = built_in.light_sapce_matrix*world_pos;
	gl_Position = cam.proj*cam.view*world_pos;
	//gl_Position = vec4(inTexCoord*2-1,0,1);
	fragPos=vec3(built_in.model*vec4(inPosition,1.0));
	fragTexCoord = inTexCoord;
	fragNormal=mat3(transpose(inverse(built_in.model)))*inNormal;

}