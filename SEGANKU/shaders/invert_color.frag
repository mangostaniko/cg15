#version 330 core

in vec2 texCoords;
out vec4 outColor;

uniform sampler2D screenColorTexture;
uniform sampler2D screenDepthTexture;

void main()
{
    outColor = vec4(vec3(1.0 - texture(screenDepthTexture, texCoords)), 1.0);
}
