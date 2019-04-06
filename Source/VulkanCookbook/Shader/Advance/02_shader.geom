#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) out vec2 geom_texcoord;

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};

void main()
{
	const float Size = 0.1;
	vec4 position = gl_in[0].gl_Position;
	
	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-Size, Size, 0.0, 0.0));
	geom_texcoord = vec2(-1.0, 1.0);
	EmitVertex();
	
	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-Size, -Size, 0.0, 0.0));
	geom_texcoord = vec2(-1.0, -1.0);
	EmitVertex();
	
	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(Size, Size, 0.0, 0.0));
	geom_texcoord = vec2(1.0, 1.0);
	EmitVertex();
	
	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(Size, -Size, 0.0, 0.0));
	geom_texcoord = vec2(1.0, -1.0);
	EmitVertex();
	
	EndPrimitive();
}