#version 330

in vec3 viewPos;

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(viewPos, 1);
} 
