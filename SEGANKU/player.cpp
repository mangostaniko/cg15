#include "player.h"

Player::CameraNavigationMode Player::cameraNavMode = FOLLOW_PLAYER;
double Player::scrollY = 0.0;

Player::Player(const glm::mat4 &matrix_, Camera *camera_, GLFWwindow *window_, const std::string &filePath)
    : Geometry(matrix_, filePath)
    , camera(camera_)
    , window(window_)
{
	// set glfw callbacks
	glfwSetScrollCallback(window, onScroll);
	glfwSetKeyCallback(window, keyCallback);

	camera->setTransform(glm::translate(glm::mat4(1.0f), getLocation()+glm::vec3(0,2,6)));  //move camera back a bit
	lastCamTransform = camera->getMatrix();
	camDirection = glm::normalize(camera->getLocation() - getLocation());
	camUp = glm::vec3(0, 1, 0);
	camRight = glm::normalize(glm::cross(camUp, camDirection));

}

Player::~Player()
{
	delete camera; camera = nullptr;
	window = nullptr;
}

void Player::update(float timeDelta)
{
	// note: camera navigation mode is toggled on tab key press, look for keyCallback
	handleNavModeChange();
	if (cameraNavMode == FOLLOW_PLAYER) {
		handleInput(window, timeDelta);

		glm::vec3 v = glm::normalize(getLocation() - camera->getLocation()) * 5.0f;
		viewProjMat = camera->getProjectionMatrix() * glm::lookAt(getLocation()-v, getLocation(), camUp);
	}
	else {
		handleInputFreeCamera(window, timeDelta);
		viewProjMat = camera->getProjectionMatrix() * camera->getViewMatrix();
	}

}

void Player::draw(Shader *shader)
{
	// pass camera view and projection matrices to shader
	GLint viewProjMatLocation = glGetUniformLocation(shader->programHandle, "viewProjMat"); // get uniform location in shader
	glUniformMatrix4fv(viewProjMatLocation, 1, GL_FALSE, glm::value_ptr(viewProjMat)); // shader location, count, transpose?, value pointer

	// pass camera position to shader
	GLint cameraPosLocation = glGetUniformLocation(shader->programHandle, "cameraPos");
	glUniform3f(cameraPosLocation, camera->getLocation().x, camera->getLocation().y, camera->getLocation().z);

	Geometry::draw(shader);
}

void Player::handleInput(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 15.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 30.0f;
	}


	// player movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) {
		translate(glm::vec3(0.0f, 0.0f, 1.0f) * timeDelta * moveSpeed, SceneObject::RIGHT);
    }
	else if (glfwGetKey(window, 'S')) {
		translate(glm::vec3(0.0f, 0.0f, -1.0f) * timeDelta * moveSpeed, SceneObject::RIGHT);
	}

	if (glfwGetKey(window, 'A')) {
		rotateY(timeDelta * glm::radians(200.f), SceneObject::RIGHT);
    }
	else if (glfwGetKey(window, 'D')) {
		rotateY(timeDelta * glm::radians(-200.f), SceneObject::RIGHT);
    }

	/*
	if (glfwGetKey(window, 'Q')) {
	    translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
	    translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	*/

	
	//// rotate camera based on mouse movement
	//// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	glm::vec3 camToTarget = camera->getLocation() - getLocation();
	glm::vec3 rightVec = glm::normalize(glm::cross(camToTarget, glm::vec3(0, 1, 0)));
	glm::mat4 rotateYaw = glm::rotate(glm::mat4(), -mouseSensitivity * (float)mouseX, glm::vec3(0, 1, 0));
	glm::mat4 rotatePitch = glm::rotate(glm::mat4(), mouseSensitivity * (float)mouseY, rightVec);

	camToTarget = glm::vec3(rotateYaw * glm::vec4(camToTarget, 0));
	camToTarget = glm::vec3(rotatePitch * glm::vec4(camToTarget, 0));
	camToTarget = camToTarget + getLocation();
	camera->setLocation(camToTarget);

	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window


	//// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;	
}

void Player::handleInputFreeCamera(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 10.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 50.0f;
	}
	
	//////////////////////////
	/// CAMERA MOVEMENT
	//////////////////////////

	// camera movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) {
		camera->translate(camera->getMatrix()[2].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'S')) {
		camera->translate(camera->getMatrix()[2].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'A')) {
		camera->translate(camera->getMatrix()[0].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'D')) {
		camera->translate(camera->getMatrix()[0].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
    }

	if (glfwGetKey(window, 'Q')) {
	    camera->translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
	    camera->translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT); // rotate around local x axis (tilt up/down)
	glm::vec3 location = camera->getLocation();
	camera->translate(-location, SceneObject::LEFT);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::LEFT); // rotate around global y at local position
	camera->translate(location, SceneObject::LEFT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void Player::onScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	scrollY += deltaY;
}

void Player::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
	    if (cameraNavMode == FOLLOW_PLAYER) {
			cameraNavMode = FREE_FLY;
		}
		else if (cameraNavMode == FREE_FLY) {
			cameraNavMode = FOLLOW_PLAYER;
		}
	}
}

void Player::handleNavModeChange()
{
	if (cameraNavMode == lastNavMode) {
		return;
	}

	glm::mat4 temp = camera->getMatrix();
	camera->setTransform(lastCamTransform);
	lastCamTransform = temp;

	lastNavMode = cameraNavMode;
}

