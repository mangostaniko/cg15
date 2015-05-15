#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
    texCoords = uv;
}
