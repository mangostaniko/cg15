#version 330 core

layout(location = 0) out vec4 outColor;

in vec3 colorVarying;

void main()
{
	outColor = vec4(colorVarying, 1);
}

