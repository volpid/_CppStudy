#version 450

layout(location = 0) in vec3 vert_normal;
layout(location = 1) in vec4 vert_texcoord;
layout(location = 2) in vec3 vert_light;

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1) 
uniform sampler2D shadowMap;

void main()
{
	float shadow = 1.0;
	vec4 shadowCoords = vert_texcoord / vert_texcoord.w;
	if (texture(shadowMap, shadowCoords.xy).r < shadowCoords.z - 0.005)
	{
		shadow = 0.5;
	}
	vec3 normalVector = normalize(vert_normal);
	vec3 lightVector = normalize(vert_light);
	float diffuseTerm = max(0.0, dot(normalVector, lightVector));
	
	frag_color = shadow * vec4(diffuseTerm) + 0.1;
}