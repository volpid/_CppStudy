#version 450

layout(vertices = 3) out;

layout(location = 0) in vec2 vert_texcoord[];
layout(location = 0) out vec2 tesc_texcoord[];

layout(set = 0, binding = 0) 
uniform UniformBuffer
{
	mat4 modelViewMatrix;
	mat4 projectionMatrix;
};

layout(set = 0, binding = 1) 
uniform sampler2D imageSampler;

void main()
{
	if (gl_InvocationID == 0)
	{
		float distance[3];
		float factors[3];
		
		for (int i = 0; i < 3; ++i)
		{
			float height = texture(imageSampler, vert_texcoord[i]).x;
			vec4 position = modelViewMatrix * (gl_in[i].gl_Position + vec4(0.0, height, 0.0, 0.0));
			distance[i] = dot(position, position);
		}
		
		factors[0] = min(distance[1], distance[2]);
		factors[1] = min(distance[2], distance[0]);
		factors[2] = min(distance[0], distance[1]);
		
		gl_TessLevelInner[0] = max(1.0, 20.0 - factors[0]);
		gl_TessLevelOuter[0] = max(1.0, 20.0 - factors[0]);
		gl_TessLevelOuter[1] = max(1.0, 20.0 - factors[1]);
		gl_TessLevelOuter[2] = max(1.0, 20.0 - factors[2]);		
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tesc_texcoord[gl_InvocationID] = vert_texcoord[gl_InvocationID];
}