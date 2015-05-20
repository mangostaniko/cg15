#version 330

layout (location = 0) in vec3 position;

// uniforms use the same value for all vertices
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 viewProjMat;

out vec3 viewPos;

void main()
{
    gl_Position = viewProjMat * modelMat * vec4(position, 1.0);
    viewPos = (viewMat * modelMat * vec4(position, 1.0)).xyz;
} 
