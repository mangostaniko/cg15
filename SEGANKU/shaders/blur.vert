#version 330 core

layout (location = 0) in vec2 position;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
    texCoord = position * 0.5f + vec2(0.5f); // transform to [0,1] range
}
