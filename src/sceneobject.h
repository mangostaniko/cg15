#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <glm/glm.hpp>

class SceneObject
{

public:
	SceneObject(const glm::mat4 &modelMatrix_);
	virtual ~SceneObject();

	virtual void update() = 0;
	virtual void draw() = 0;

	glm::mat4 modelMatrix;

};

#endif // SCENEOBJECT_H
