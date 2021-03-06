#version 450

layout(location = 0) in vec4 app_position;
layout(location = 1) in vec4 app_color;

layout(location = 0) out vec4 vert_color;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	gl_Position = modelViewMatrix * app_position;
	vert_color = app_color;
}