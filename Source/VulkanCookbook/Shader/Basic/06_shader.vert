#version 450

layout(location = 0) in vec4 app_position;
layout(location = 0) out float vert_color;

layout(set = 0, binding = 0)
uniform UniformBuffer
{
	mat4 ProjectionMatrix;
};

void main()
{
	gl_Position = ProjectionMatrix * app_position;
	vert_color = app_position.z * 0.5 + 0.5;
}