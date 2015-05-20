#version 330

in vec3 viewPos;

layout (location = 0) out vec3 outColor;

void main()
{
    outColor = viewPos;
} 
