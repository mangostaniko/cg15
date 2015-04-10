#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <glm/glm.hpp>

class SceneObject
{

public:
	SceneObject(const glm::mat4 &modelMatrix_);
	virtual ~SceneObject();

	/**
	 * @brief update the state of the SceneObject
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	virtual void update(float timeDelta) = 0;

	/**
	 * @brief draw the SceneObject
	 */
	virtual void draw() = 0;

	glm::mat4 modelMatrix;

};

#endif // SCENEOBJECT_H
