#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// these will be interpolated by the gpu
// the interpolated values can be accessed by same name in fragment shader
out vec3 worldNormal;
out vec2 texCoord;

// uniforms use the same value for all vertices
uniform mat4 modelMat;
uniform mat4 viewProjMat;

void main()
{
	worldNormal = (modelMat * vec4(normal, 0)).xyz; // normals always have origin at 0, thus translational factor is 0
	texCoord = uv;

	gl_Position = viewProjMat * modelMat * vec4(position, 1);
}

