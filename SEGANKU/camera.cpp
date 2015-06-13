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

bool Camera::checkSphereInFrustum(const glm::vec3 &sphereCenter, const glm::vec3 &sphereFarthestPoint)
{
	// get sphere into clip space and then into normalized device coordinates through perspective division,
	// i.e. division by w which after the perspective projection stores the depth component z,
	// such that all 6 view frustum planes are simply at distance 1 or -1 from the origin
	glm::vec4 sphereCenterClipSpace = getProjectionMatrix() * getViewMatrix() * glm::vec4(sphereCenter, 1);
	glm::vec4 sphereFarthestPointClipSpace = getProjectionMatrix() * getViewMatrix() * glm::vec4(sphereFarthestPoint, 1);
	sphereCenterClipSpace /= sphereCenterClipSpace.w;
	sphereFarthestPointClipSpace /= sphereFarthestPointClipSpace.w;

	float sphereRadiusClipSpace = glm::length(sphereFarthestPointClipSpace - sphereCenterClipSpace);

	// TODO
	// check if the sphere lies beyond any of the 6 view frustum planes
	// note that we use a greater margin than 1, to compensate for the very rough bounding sphere approximation
	float planesDistance = 1.f;
	if ((sphereCenterClipSpace.x > planesDistance && sphereCenterClipSpace.x - sphereRadiusClipSpace > planesDistance) || (sphereCenterClipSpace.x < -planesDistance && sphereCenterClipSpace.x + sphereRadiusClipSpace < -planesDistance)) {
		return false;
	}
	if ((sphereCenterClipSpace.y > planesDistance && sphereCenterClipSpace.y - sphereRadiusClipSpace > planesDistance) || (sphereCenterClipSpace.y < -planesDistance && sphereCenterClipSpace.y + sphereRadiusClipSpace < -planesDistance)) {
		return false;
	}

	return true;
}

