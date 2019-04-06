#version 450

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec3 vert_normal;

layout(location = 0) out vec4 frag_color;

layout(push_constant)
uniform LightParameter
{
	vec4 position;
} Light;

void main()
{
	vec3 normalVector = normalize(vert_normal);
	vec3 lightVector = normalize(Light.position.xyz - vert_position);
	float diffuseTerm = max(0.0, dot(normalVector, lightVector));
	
	frag_color = vec4(diffuseTerm + 0.1);
	
	if (diffuseTerm > 0.0)
	{
		vec3 viewVector = normalize(vec3(0.0, 0.0, 0.0) - vert_position.xyz);
		vec3 halfVector = normalize(viewVector + lightVector);
		
		float shiness = 30.0;
		float specularTerm = pow(dot(viewVector, halfVector), shiness);
		
		frag_color += vec4(specularTerm);
	}
}