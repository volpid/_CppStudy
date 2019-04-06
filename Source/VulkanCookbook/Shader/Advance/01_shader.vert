#version 450

layout(location = 0) in vec4 app_position;
layout(location = 0) out vec3 vert_texcoord;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	vec3 position = mat3(modelViewMatrix) * app_position.xyz;
	gl_Position = (projectionMatrix * vec4(position, 0.0)).xyzz;
	
	vert_texcoord = app_position.xyz;
}