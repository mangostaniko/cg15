#version 330 core

layout(location = 0) out vec4 outColor;

// these are vertex shader variables interpolated by the gpu
in vec3 worldNormal; // range [-1, 1]
in vec2 texCoord;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

void main()
{
	outColor = vec4(texture(diffuseTexture, texCoord).rgb, 1);
}

