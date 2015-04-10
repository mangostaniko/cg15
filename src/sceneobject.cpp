#include "sceneobject.h"
#include "glm/gtx/transform.hpp"

SceneObject::SceneObject(const glm::mat4 &modelMatrix_)
	: modelMatrix(modelMatrix_)
{
	inverseMatrix = glm::inverse(modelMatrix_);
}

SceneObject::~SceneObject()
{
	modelMatrix = glm::mat4();
	inverseMatrix = glm::mat4();
}


const glm::mat4& SceneObject::getMatrix() const
{
	return modelMatrix;
}

const glm::mat4& SceneObject::getInverseMatrix() const
{
	return inverseMatrix;
}

void SceneObject::setTransform(const glm::mat4 &matrix_) {
	modelMatrix = matrix_;
	inverseMatrix = glm::inverse(modelMatrix);
}

void SceneObject::applyTransformation(const glm::mat4 &transform_, const glm::mat4 &inverse_, Order multOrder)
{
	if (multOrder == LEFT) {
		modelMatrix		= transform_*modelMatrix;
		inverseMatrix	= inverseMatrix*inverse_;
	}
	else {
		modelMatrix		= modelMatrix * transform_;
		inverseMatrix	= inverse_*inverseMatrix;
	}
}

void SceneObject::rotateX(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(radians, glm::vec3(1.0f, 0.0f, 0.0f)), glm::rotate(-radians, glm::vec3(1.0f, 0.0f, 0.0f)), multOrder);
}

void SceneObject::rotateY(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(radians, glm::vec3(0.0f, 1.0f, 0.0f)), glm::rotate(-radians, glm::vec3(0.0f, 1.0f, 0.0f)), multOrder);
}

void SceneObject::rotateZ(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(radians, glm::vec3(0.0f, 0.0f, 1.0f)), glm::rotate(-radians, glm::vec3(0.0f, 0.0f, 1.0f)), multOrder);
}

void SceneObject::rotate(float radians, Order multOrder, const glm::vec3 &axis_)
{
	applyTransformation(glm::rotate(radians, axis_), glm::rotate(-radians, axis_), multOrder);
}

void SceneObject::translate(const glm::vec3 &t_, Order multOrder)
{
	applyTransformation(glm::translate(t_), glm::translate(-t_), multOrder);
}

void SceneObject::scale(const glm::vec3 &s_, Order multOrder)
{
	applyTransformation(glm::scale(s_), glm::scale(1.0f / s_), multOrder);
}