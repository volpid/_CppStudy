#version 450

layout(location = 0) in vec3 vert_position;

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1) 
uniform samplerCube cubeMap;

void main()
{
	frag_color = texture(cubeMap, vert_position);
}