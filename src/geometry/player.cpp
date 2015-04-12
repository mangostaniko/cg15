#include "player.h"

Player::CameraNavigationMode Player::cameraNavMode = FOLLOW_PLAYER;
double Player::scrollY = 0.0;

Player::Player(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_, Camera *camera_, GLFWwindow *window_)
    : Cube(matrix_, shader_, texture_)
{
	camera = camera_;
	window = window_;

	// set glfw callbacks
	glfwSetScrollCallback(window, onScroll);
	glfwSetKeyCallback(window, keyCallback);

	camera->translate(glm::vec3(0, 0, 10), SceneObject::LEFT); // move camera back a bit
}

Player::~Player()
{
	delete camera; camera = nullptr;
	window = nullptr;
}

void Player::update(float timeDelta)
{
	// note: camera navigation mode is toggled on tab key press, look for keyCallback
	if (cameraNavMode == FOLLOW_PLAYER) {
		handleInput(window, timeDelta);
	}
	else if (cameraNavMode == FREE_FLY) {
		handleInputFreeCamera(window, timeDelta);
	}

	camera->update(timeDelta);
	//camera->lookAt(glm::vec3(cubes.at(6)->getLocation())); // broken

	// pass camera view and projection matrices to shader
	glm::mat4 viewProjMat = camera->getProjectionMatrix() * camera->getViewMatrix();
	GLint viewProjMatLocation = glGetUniformLocation(shader->programHandle, "viewProjMat"); // get uniform location in shader
	glUniformMatrix4fv(viewProjMatLocation, 1, GL_FALSE, glm::value_ptr(viewProjMat)); // shader location, count, transpose?, value pointer

}

void Player::handleInput(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 20.0f;
	}

	// lock camera to player
	// for some reason it only works of we modify the player location, not the camera location
	setLocation(camera->getLocation());
	translate(-5.0f * glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]), SceneObject::RIGHT);

	// camera movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) {
		camera->translate(glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]) * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'S')) {
		camera->translate(glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]) * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'A')) {
		camera->translate(glm::vec3(camera->getMatrix()[0][0], 0, camera->getMatrix()[0][2]) * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'D')) {
		camera->translate(glm::vec3(camera->getMatrix()[0][0], 0, camera->getMatrix()[0][2]) * timeDelta * moveSpeed, SceneObject::LEFT);
    }

	if (glfwGetKey(window, 'Q')) {
	    camera->translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
	    camera->translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}

	// lock camera to player
	// for some reason it only works of we modify the player location, not the camera location
	setLocation(camera->getLocation());
	translate(-5.0f * glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]), SceneObject::LEFT);

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::LEFT);
	camera->rotateX(mouseSensitivity * (float)mouseY, SceneObject::LEFT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(20.0f)) fieldOfView = glm::radians(20.0f);
	if (fieldOfView > glm::radians(60.0f)) fieldOfView = glm::radians(60.0f);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void Player::handleInputFreeCamera(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 20.0f;
	}

	//////////////////////////
	/// CAMERA MOVEMENT
	//////////////////////////

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::RIGHT);
	camera->rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

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

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(20.0f)) fieldOfView = glm::radians(20.0f);
	if (fieldOfView > glm::radians(60.0f)) fieldOfView = glm::radians(60.0f);
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
