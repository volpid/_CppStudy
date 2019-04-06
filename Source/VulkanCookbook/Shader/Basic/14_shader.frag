#version 450

layout(location = 0) in vec2 vert_texcoord;
layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 0) 
uniform sampler2D Image;

void main()
{
	float grey = dot(texture(Image, vert_texcoord).rgb, vec3(0.3, 0.6, 0.1));
	frag_color = vec4(grey);
}