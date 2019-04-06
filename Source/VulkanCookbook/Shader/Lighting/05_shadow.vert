#version 450

layout(location = 0) in vec4 app_position;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 shadowModelViewMatrix;
	mat4 sceneModelViewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	gl_Position = projectionMatrix * shadowModelViewMatrix * app_position;
}