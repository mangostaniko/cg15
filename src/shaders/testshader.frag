#version 330 core

layout(location = 0) out vec4 outColor;

// these are vertex shader variables interpolated by the gpu
in vec3 worldNormal; // range [-1, 1]
in vec2 texCoord;

uniform sampler2D colorTexture;

void main()
{
	outColor = vec4(1.2 - texture(colorTexture, texCoord).rgb, 1) * vec4(0.5 + worldNormal/2, 1);
}

