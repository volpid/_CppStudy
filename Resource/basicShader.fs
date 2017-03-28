#version 120

varying vec2 texCoors;
varying vec3 normal0;

uniform sampler2D diffuse;

void main()
{
	vec3 lightDirection = vec3(0.0f, 0.0f, 1.0f);
	gl_FragColor = texture2D(diffuse, texCoors) 
		* clamp(dot(-lightDirection, normal0), 0.0f, 1.0f);
}