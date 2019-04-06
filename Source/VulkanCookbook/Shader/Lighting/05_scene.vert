#version 450

layout(location = 0) in vec4 app_position;
layout(location = 1) in vec3 app_normal;

layout(location = 0) out vec3 vert_normal;
layout(location = 1) out vec4 vert_texcoord;
layout(location = 2) out vec3 vert_light;

layout(push_constant)
uniform LightParameters
{
	vec4 position;
} Light;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 shadowModelViewMatrix;
	mat4 sceneModelViewMatrix;
	mat4 projectionMatrix;
};

const mat4 bias = mat4
(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main()
{
	gl_Position = projectionMatrix * sceneModelViewMatrix * app_position;
	
	vert_normal = mat3(sceneModelViewMatrix) * app_normal;
	vert_texcoord = bias * projectionMatrix * shadowModelViewMatrix * app_position;
	vert_light = (sceneModelViewMatrix * vec4(Light.position.xyz, 0.0)).xyz;
}