#version 450

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec2 vert_texcoord;
layout(location = 2) in vec3 vert_normal;
layout(location = 3) in vec3 vert_tangent;
layout(location = 4) in vec3 vert_bitangent;

layout(location = 0) out vec4 frag_color;

layout(push_constant)
uniform LightParameter
{
	vec4 position;
} Light;

layout(set = 0, binding = 1) 
uniform sampler2D ImageSamper;

void main()
{
	vec3 normal = 2 * texture(ImageSamper, vert_texcoord).rgb - 1.0;
	
	vec3 normalVector = normalize(mat3(vert_tangent, vert_bitangent, vert_normal) * normal);
	vec3 lightVector = normalize(Light.position.xyz - vert_position);
	float diffuseTerm = max(0.0, dot(normalVector, lightVector));
	
	frag_color = vec4(diffuseTerm + 0.1);
	
	if (diffuseTerm > 0.0)
	{
		vec3 viewVector = normalize(-vert_position.xyz);
		vec3 halfVector = normalize(viewVector + lightVector);
		
		float shiness = 30.0;
		float specularTerm = pow(dot(viewVector, halfVector), shiness);
		
		frag_color += vec4(specularTerm);
	}
}