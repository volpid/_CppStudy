#version 120

attribute vec3 position;
attribute vec2 coords;
attribute vec3 normal;

varying vec2 texCoors;
varying vec3 normal0;

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(position, 1.0f);
	texCoors = coords;
	normal0 = (transform * vec4(normal, 0.0f)).xyz;
}