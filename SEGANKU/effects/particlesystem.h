#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vector>
#include <memory>
#include <algorithm>

#include "../sceneobject.h"
#include "../shader.h"
#include "../texture.h"

struct Particle
{
    glm::vec3 pos, velocity;
	GLfloat mass = 1.0f;
	GLfloat cameraDistance = 0.0f;
    GLfloat timeToLive = 2000.0f; // seconds

	// for sorting in back to front drawing order.
	// this is needed for alpha blending since zbuffer test rejects fragments that lie behind,
	// but their color data is needed for blending.
	bool operator<(Particle& other)
	{
		return this->cameraDistance > other.cameraDistance;
	}

};

class ParticleSystem : SceneObject
{
	GLuint vao;
	GLuint particleQuadVBO;
	GLuint particleInstanceDataVBO;

	Shader *particleShader = nullptr;
	Texture *particleTexture = nullptr;

	const unsigned int MAX_PARTICLE_COUNT = 1000;
	const float SPAWN_RATE = 200.0f; // how many particles to spawn per second

	float secondsSinceLastSpawn = 0.0f; // time since last spawned particle, in seconds


	std::vector<std::shared_ptr<Particle>> particles;

public:

	ParticleSystem(const glm::mat4 &matrix_, const std::string &texturePath);
	~ParticleSystem();

	/**
	 * @brief update the particles in the particle system
	 * @param timeDelta the time since the last frame
	 */
	void update(float timeDelta, glm::vec3 cameraPos);

	/**
	 * @brief draw the particles in the particle system
	 */
	void draw(const glm::mat4 &viewMat, const glm::mat4 &projMat);
};

#endif // PARTICLESYSTEM_H
