#version 450

layout(location = 0) in vec3 vert_texcoord;
layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 1)
uniform samplerCube cubemap;

void main()
{
	frag_color = texture(cubemap, vert_texcoord);
}