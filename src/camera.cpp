#include "camera.h"

#include <iostream>

Camera::Camera(const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_)
	: SceneObject(matrix_)
    , fieldOfView(fieldOfView_)
    , aspectRatio(aspectRatio_)
    , nearPlane(nearPlane_)
    , farPlane(farPlane_)
{

}

Camera::Camera(const glm::mat4 &matrix_)
	: SceneObject(matrix_)
    , fieldOfView(glm::pi<float>()/3)
    , aspectRatio(4.0f/3.0f)
    , nearPlane(0.2f)
    , farPlane(100.0f)
{

}

Camera::~Camera()
{

}


void Camera::update(float timeDelta)
{
	/* TODO: try input handling here */

	//rotateY(timeDelta, LEFT);
}

void Camera::draw()
{
	/* left empty for now */
}


glm::mat4 Camera::getViewMatrix() const
{
	return getInverseMatrix();
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
}

float Camera::getFieldOfView() const
{
	return fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView_)
{
	fieldOfView = fieldOfView_;
}

float Camera::getAspectRatio() const
{
	return aspectRatio;
}

void Camera::setAspectRatio(float aspectRatio_)
{
	aspectRatio = aspectRatio_;
}

float Camera::getNearPlane() const
{
	return nearPlane;
}

void Camera::setNearPlane(float nearPlane_)
{
	nearPlane = nearPlane_;
}

float Camera::getFarPlane() const
{
	return farPlane;
}

void Camera::setFarPlane(float farPlane_)
{
	farPlane = farPlane_;
}

void Camera::lookAt(const glm::vec3 &target)
{
	glm::vec3 up = getMatrix()[1].xyz(); // assume the camera up vector to be the y axis

	glm::vec3 Z = getLocation() - target; // look in opposite direction to camera, which looks in -z
	Z = glm::normalize(Z);
	glm::vec3 X = glm::cross(up, Z);
	glm::vec3 Y = glm::cross(X, Z); // y is still the up vector, but kind of 'rotated' around x to fit with z

	X = glm::normalize(X);
	Y = glm::normalize(Y);

	glm::mat4 lookAtMat;
	lookAtMat[0] = glm::vec4(X, 0);
	lookAtMat[1] = glm::vec4(Y, 0);
	lookAtMat[2] = glm::vec4(Z, 0);
	lookAtMat[3] = glm::vec4(0, 0, 0, 1);

	applyTransformation(lookAtMat, glm::inverse(lookAtMat), LEFT);

	std::cout << matrixToString(getMatrix()) << std::endl;
	std::cout << matrixToString(getInverseMatrix()) << std::endl;


}

