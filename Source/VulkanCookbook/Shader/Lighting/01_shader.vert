#version 450

layout(location = 0) in vec4 app_position;
layout(location = 1) in vec3 app_normal;
layout(location = 0) out float vert_color;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};


void main()
{
	gl_Position = projectionMatrix * modelViewMatrix* app_position;
	vec3 normal = mat3(modelViewMatrix) * app_normal;
	
	vert_color = max(0.0, dot(normal, vec3(0.58, 0.58, 0.58))) + 0.1;
}