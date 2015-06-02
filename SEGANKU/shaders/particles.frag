#version 330 core

in vec2 texCoord;
in float timeToLive; // used to fade alpha or change color
layout(location = 0) out vec4 outColor;

uniform sampler2D particleTexture;
uniform vec3 color;

void main()
{
    outColor = vec4(texture(particleTexture, texCoord).rgba);
}
