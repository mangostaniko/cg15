#version 330 core

layout(location = 0) out vec4 outColor;

// these are vertex shader variables interpolated by the gpu
in vec3 worldNormal; // range [-1, 1]

void main()
{
	outColor = vec4(worldNormal, 1);
}

