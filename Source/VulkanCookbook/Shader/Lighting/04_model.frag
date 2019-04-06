#version 450

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec3 vert_normal;

layout(location = 0) out vec4 frag_color;

layout(push_constant)
uniform LightParameter
{
	vec4 position;
} Camera;

layout(set = 0, binding = 1) 
uniform samplerCube cubeMap;

void main()
{
	vec3 viewVector = vert_position - Camera.position.xyz;
	
	float angle = smoothstep(0.3, 0.7, dot(normalize(-viewVector), vert_normal));
	
	vec3 reflectVector = reflect(viewVector, vert_normal);
	vec4 reflectColor = texture(cubeMap, reflectVector);
	vec3 refractVector = refract(viewVector, vert_normal, 0.3);
	vec4 refractColor = texture(cubeMap, refractVector);
	
	frag_color = mix(reflectColor, refractColor, angle);
}