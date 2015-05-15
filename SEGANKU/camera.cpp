#include "camera.h"

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
	/* left empty */
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
	setTransform(glm::lookAt(getLocation(), target, glm::vec3(0, 1, 0)));
}

