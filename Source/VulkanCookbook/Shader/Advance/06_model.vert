#version 450

layout(location = 0) in vec4 app_position;
layout(location = 1) in vec3 app_normal;

layout(location = 0) out vec3 vert_position;
layout(location = 1) out vec3 vert_normal;

layout(set = 0, binding = 0)
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	vert_position = app_position.xyz;
	vert_normal = app_normal;
	
	gl_Position = projectionMatrix * modelViewMatrix * app_position;
}