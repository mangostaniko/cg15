#ifndef CAMERA_H
#define CAMERA_H

#define GLM_SWIZZLE

#include "sceneobject.h"

class Camera : public SceneObject
{
	float fieldOfView;
	float aspectRatio;
	float nearPlane;
	float farPlane;

public:
	Camera(const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_);
	Camera(const glm::mat4 &matrix_);
	virtual ~Camera();

	virtual void update(float timeDelta);
	virtual void draw();

	/**
	 * @brief get the view matrix, i.e. the inverse camera model matrix
	 * in which the camera is at the origin and looking in -z direction
	 * @return the view matrix
	 */
	glm::mat4 getViewMatrix() const;

	/**
	 * @brief get the current projection matrix defining the view frustum
	 * calculated from the parameters stored
	 * @return the current projection matrix defining the view frustum
	 */
	glm::mat4 getProjectionMatrix() const;

	/**
	 * @brief get field of view of the viewing frustum
	 * @return the field of view angle in radians
	 */
	float getFieldOfView() const;

	/**
	 * @brief set field of view of the viewing frustum
	 * @param fieldOfView_ the new field of view angle in radians
	 */
	void setFieldOfView(float fieldOfView_);

	/**
	 * @brief get the aspect ratio of the viewing frustum (width / height)
	 * @return the aspect ratio (width / height)
	 */
	float getAspectRatio() const;

	/**
	 * @brief set the aspect ratio of the viewing frustum (width / height)
	 * @param aspectRatio_ the new aspect ratio (width / height)
	 */
	void setAspectRatio(float aspectRatio_);

	/**
	 * @brief get distance from camera to near plane of the viewing frustum
	 * @return the distance form camera to near plane
	 */
	float getNearPlane() const;

	/**
	 * @brief set distance from camera to near plane of the viewing frustum
	 * @param nearPlane_ the new distance form camera to near plane
	 */
	void setNearPlane(float nearPlane_);

	/**
	 * @brief get distance from camera to near plane of the viewing frustum
	 * @return the distance form camera to near plane
	 */
	float getFarPlane() const;

	/**
	 * @brief set distance from camera to far plane of the viewing frustum
	 * @return farPlane_ the new distance form camera to far plane
	 */
	void setFarPlane(float farPlane_);

	/**
	 * @brief rotate camera to look at a given point
	 * note: implemented here to learn a bit, but basically just like glm::lookAt
	 * @param target the target point to look at. this should not be the camera location.
	 */
	void lookAt(const glm::vec3 &target);


};

#endif // CAMERA_H
