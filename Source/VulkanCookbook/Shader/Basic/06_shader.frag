#version 450

layout(location = 0) in float ver_color;
layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = vec4(ver_color, ver_color, ver_color, ver_color);	
}