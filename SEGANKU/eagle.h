#ifndef EAGLE_H
#define EAGLE_H

#define GLM_SWIZZLE

#include <btBulletDynamicsCommon.h>
#include "geometry.h"

enum EagleState
{
	CIRCLING    = 0,
	ATTACKING   = 1,
	RETREATING  = 2
};

class Eagle : public Geometry
{
	// PHYSICS
	btCollisionShape *playerShape;
	btRigidBody *playerBody;
	btDefaultMotionState *motionState;

	// GAMEPLAY
	// all durations given in seconds

	const float TARGET_REACH_RADIUS = 10.0f;
	const float ATTACK_WAIT_TIME_MIN = 10.0f; // min time after which eagle will attempt to attack player
	const float ATTACK_WAIT_TIME_MAX = 20.0f; // max time after which eagle will attempt to attack player
	glm::mat4 eagleInitTransform;

	EagleState state = CIRCLING;

	float totalTimePassed = 0;
	float timeSinceLastAttack = 0;
	float timeIntervalToNextAttack = ATTACK_WAIT_TIME_MIN;


public:
	Eagle(const glm::mat4 &matrix, const std::string &filePath);
	~Eagle();

	virtual void update(float timeDelta, const glm::vec3 &targetPos, bool targetHidden, bool targetDefenseActive);

	/**
	 * @brief return the current state of behaviour of the eagle
	 * @return the current state of behaviour of the eagle
	 */
	EagleState getState();

private:
};

#endif // EAGLE_H
