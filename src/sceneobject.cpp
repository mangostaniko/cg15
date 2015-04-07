#include "sceneobject.h"

SceneObject::SceneObject(const glm::mat4 &modelMatrix_)
	: modelMatrix(modelMatrix_)
{
}

SceneObject::~SceneObject()
{
}

