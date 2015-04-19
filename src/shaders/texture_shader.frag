#version 330 core

layout(location = 0) out vec4 outColor;

// these are vertex shader variables interpolated by the gpu
in vec3 P; // world space position
in vec3 N; // world space normal
in vec2 texCoord; // interpolated texture coordinates

uniform vec3 cameraPos;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

const vec3 lightPos = vec3(40, 60, 50); // light position
const vec3 lightColor = vec3(1.0f, 0.9f, 0.7f);
const float ambientFactor = 0.3f;

// TODO: these should be made uniforms and defined by object materials
const float shininess = 20.0f;

void main()
{
	vec3 diffuseColor = texture(diffuseTexture, texCoord).rgb;
	vec3 specularColor = vec3(1.0f); //texture(specularTexture, texCoord).rgb;

	vec3 N = normalize(N);
	vec3 L = normalize(lightPos - P); // light vector (point to light)
	vec3 V = normalize(cameraPos - P);

	vec3 ambient = ambientFactor * lightColor * diffuseColor;
	vec3 diffuse = max(dot(N, L), 0.0f) * lightColor * diffuseColor;

	vec3 H = normalize(L + V); // half vector of light and view vectors
	// note that N is the half vector of light vector and its specular reflection
	vec3 specular = pow(max(dot(H, N), 0.0f), shininess) * lightColor * diffuseColor;

	outColor = vec4(ambient + diffuse + specular, 1);
}
