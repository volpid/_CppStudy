#version 450

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec3 vert_normal;

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1)
uniform samplerCube cubemap;

layout(push_constant)  
uniform LightParameter
{
	vec4 position;
} camera;

void main()
{
	vec3 viewVector = vert_position - camera.position.xyz;
	
	float angle = smoothstep(0.3, 0.7, dot(normalize(-viewVector), vert_normal));
	
	vec3 reflectVector = reflect(viewVector, vert_normal);
	vec4 reflectColor = texture(cubemap, reflectVector);
	
	vec3 refractVector = refract(viewVector, vert_normal, 0.3);
	vec4 refractColor = texture(cubemap, refractVector);
	
	frag_color = mix(reflectColor, reflectColor, angle) * vec4(1.0, 0.6, 0.2, 1.0);
}