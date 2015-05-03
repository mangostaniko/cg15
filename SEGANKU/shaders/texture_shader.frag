#version 330 core

struct Material {
	
	sampler2D diffuse;
	vec3 specular;
	float shininess;

	// specular and normal maps currently not used
	//sampler2D specular;
	//sampler2D normal;
};

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 0) out vec4 outColor;

in vec3 P;			
in vec3 N;			
in vec2 texCoord;

uniform vec3 cameraPos;
uniform Material material;
uniform Light light;

void main()
{
	// Normalize normal, light and view vectors
	vec3 normal = normalize(N);
	vec3 lightDir = normalize(light.position - P); 
	vec3 viewDir = normalize(cameraPos - P);
	
	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;

	// Diffuse
	vec3 diffuseColor = texture(material.diffuse, texCoord).rgb;	
	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * diffuseColor * light.diffuse;
	
	// Specular
	vec3 halfVec = normalize(lightDir + viewDir); // half vector of light and view vectors
	vec3 specularColor = vec3(1.0f); //texture(specularTexture, texCoord).rgb;	
	vec3 specular = pow(max(dot(halfVec, normal), 0.0f), material.shininess) * light.specular * material.specular;

	outColor = vec4(ambient + diffuse + specular, 1);
}

