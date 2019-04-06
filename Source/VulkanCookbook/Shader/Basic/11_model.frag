#version 450

layout(location = 0) in vec3 vert_normal;
layout(location = 0) out vec4 frag_color;

void main()
{
	float diffuseLight = max(0, dot(vert_normal, vec3(0.5, 0.5, 0.5)));
	frag_color = vec4(diffuseLight) + 0.1;
}