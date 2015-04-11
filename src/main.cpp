#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "shader.h"
#include "sceneobject.h"
#include "camera.h"
#include "geometry/cube.h"


void init(GLFWwindow *window);
void update(float timeDelta);
void draw();
void cleanup();
void handleInput(GLFWwindow *window, float timeDelta);
void handleInputFreeCamera(GLFWwindow *window, float timeDelta);

GLFWwindow *window;

Camera *camera;
Shader *shader;
Texture *texture;
SceneObject *player;
std::vector<std::shared_ptr<SceneObject>> cubes;

float degToRad(float deg);
void frameBufferResize(GLFWwindow *window, int width, int height);
void onScroll(GLFWwindow *window, double deltaX, double deltaY);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

double scrollY = 0.0;

enum CameraNavigationMode
{
	FOLLOW_PLAYER,
	FREE_FLY
} cameraNavMode(FOLLOW_PLAYER);

int main(int argc, char **argv)
{
	// HANDLE COMMAND LINE PARAMETERS

	int windowWidth = 800;
	int windowHeigth = 600;
	int refresh_rate = 60;
	bool fullscreen = 0;

	if (argc == 1) {
		// no parameters specified, continue with default values

	} else if (argc != 4 || (std::stringstream(argv[1]) >> windowWidth).fail() || (std::stringstream(argv[2]) >> windowHeigth).fail() || (std::stringstream(argv[3]) >> fullscreen).fail()) {
		// if parameters are specified, must conform to given format

		std::cout << "USAGE: <resolution width> <resolution height> <fullscreen? 0/1>\n";
		exit(EXIT_FAILURE);
	}

	// INIT WINDOW AND OPENGL CONTEXT

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *videoMode = glfwGetVideoMode(monitor);

	window = nullptr;
	window = glfwCreateWindow(windowWidth, windowHeigth, "SEGANKU", (fullscreen ? monitor : NULL), NULL);
	if (!window)
	{
		std::cerr << "ERROR: Failed to open GLFW window.\n";
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// center window on screen
	glfwSetWindowPos(window, videoMode->width/2 - windowWidth/2, videoMode->height/2 - windowHeigth/2);

	glfwMakeContextCurrent(window);

	// capture mouse pointer and hide it
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);

	//glClearColor(1, 1, 1, 1); // white
	glClearColor(0.68f, 0.78f, 1.0f, 1.0f); // warm sky blue
	//glClearColor(0.11f, 0.1f, 0.14f, 1.0f); // dark grey
	glViewport(0, 0, windowWidth, windowHeigth);

	// print OpenGL version
	std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << glewGetErrorString(err);
	}

	// set callbacks
	glfwSetFramebufferSizeCallback(window, frameBufferResize);
	glfwSetScrollCallback(window, onScroll);
	glfwSetKeyCallback(window, keyCallback);

	init(window);


	//////////////////////////
	/// MAIN LOOP
	//////////////////////////

	double time = 0.0;
	double lastTime = 0.0;
	double deltaT = 0.0;
	double deltaShowFpsTime = 0.0;

	bool running = true;

	while (running && !glfwWindowShouldClose(window)) {

		//////////////////////////
		/// HANDLE EVENTS
		//////////////////////////

		time = glfwGetTime(); // seconds
		deltaT = time - lastTime;
		lastTime = time;

		// print framerate around every second (console output is costly)
		deltaShowFpsTime += deltaT;
		if (deltaShowFpsTime > 1.0) {
			deltaShowFpsTime = 0;
			std::cout << "fps: " << (int)(1/deltaT) << std::endl;
		}

		// note: camera navigation mode is toggled on tab key press, look for keyCallback
		if (cameraNavMode == FOLLOW_PLAYER) {
			handleInput(window, deltaT);
		}
		else if (cameraNavMode == FREE_FLY) {
			handleInputFreeCamera(window, deltaT);
		}


		//////////////////////////
		/// UPDATE
		//////////////////////////

		update(deltaT);


		//////////////////////////
		/// DRAW
		//////////////////////////

		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw();

		// end the current frame (swaps the front and back buffers)
		glfwSwapBuffers(window);

		glfwPollEvents();

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR) {
			// handle errors
			std::cerr << "ERROR: OpenGL Error " << glErr << std::endl;
		}

		running = !glfwGetKey(window, GLFW_KEY_ESCAPE);

	}

	// release resources
	cleanup();

	glfwTerminate();
	exit(EXIT_SUCCESS); // for system independent success code
	return 0; // to silence compiler warnings
}



void init(GLFWwindow *window)
{
	// enable z buffer test
	glEnable(GL_DEPTH_TEST);


	// INIT CAMERA

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	camera = new Camera(glm::mat4(1.0f), degToRad(60.0f), width/(float)height, 0.2f, 60.0f); // mat, fov, aspect, znear, zfar
	camera->translate(glm::vec3(0, 0, 10), SceneObject::LEFT);


	// INIT SHADERS

	shader = new Shader("../src/shaders/testshader.vert", "../src/shaders/testshader.frag");
	shader->useShader();

	// pass camera view and projection matrices to shader
	glm::mat4 viewProjMat = camera->getProjectionMatrix() * camera->getViewMatrix();
	GLint viewProjMatLocation = glGetUniformLocation(shader->programHandle, "viewProjMat"); // get uniform location in shader
	glUniformMatrix4fv(viewProjMatLocation, 1, GL_FALSE, glm::value_ptr(viewProjMat)); // shader location, count, transpose?, value pointer


	// INIT TEXTURES
	// note: use 8 bit RGB, no alpha channel

	//texture = new Texture("../data/textures/uv_debug_tex.png");
	texture = new Texture("../data/textures/skunk_drawing.jpg");


	// INIT OBJECTS

	for (int i = 0; i < 3; ++i) {
		cubes.push_back(std::make_shared<Cube>(glm::translate(glm::mat4(1.0f), glm::vec3(-2 + i*2, 0, 0)), shader, texture));
	}
	for (int i = 0; i < 3; ++i) {
		cubes.push_back(std::make_shared<Cube>(glm::translate(glm::mat4(1.0f), glm::vec3(-2 + i*2, 2, 0)), shader, texture));
	}
	for (int i = 0; i < 3; ++i) {
		cubes.push_back(std::make_shared<Cube>(glm::translate(glm::mat4(1.0f), glm::vec3(-2 + i*2, -2, 0)), shader, texture));
	}

	player = new Cube(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 6)), shader, texture);

}

void update(float timeDelta)
{
	camera->update(timeDelta);
	//camera->lookAt(glm::vec3(cubes.at(6)->getLocation())); // broken

	// pass camera view and projection matrices to shader
	glm::mat4 viewProjMat = camera->getProjectionMatrix() * camera->getViewMatrix();
	GLint viewProjMatLocation = glGetUniformLocation(shader->programHandle, "viewProjMat"); // get uniform location in shader
	glUniformMatrix4fv(viewProjMatLocation, 1, GL_FALSE, glm::value_ptr(viewProjMat)); // shader location, count, transpose?, value pointer

	// update cubes
	for (unsigned i = 0; i < cubes.size(); ++i) {
		cubes[i]->update(((i%2)-0.5) * (i%cubes.size()/2+1) * timeDelta);
	}

}

void draw()
{
	player->draw();

	for (unsigned i = 0; i < cubes.size(); ++i) {
		cubes[i]->draw();
	}

}

void cleanup()
{
	delete shader; shader = nullptr;
	delete texture; texture = nullptr;
	delete camera; camera = nullptr;
	delete player; player = nullptr;
}

void handleInput(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 20.0f;
	}

	// lock camera to player
	// for some reason it only works of we modify the player location, not the camera location
	player->setLocation(camera->getLocation());
	player->translate(-5.0f * glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]), SceneObject::RIGHT);

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
	player->setLocation(camera->getLocation());
	player->translate(-5.0f * glm::vec3(camera->getMatrix()[2][0], 0, camera->getMatrix()[2][2]), SceneObject::LEFT);

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
	if (fieldOfView < degToRad(20.0f)) fieldOfView = degToRad(20.0f);
	if (fieldOfView > degToRad(60.0f)) fieldOfView = degToRad(60.0f);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void handleInputFreeCamera(GLFWwindow *window, float timeDelta)
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
	if (fieldOfView < degToRad(20.0f)) fieldOfView = degToRad(20.0f);
	if (fieldOfView > degToRad(60.0f)) fieldOfView = degToRad(60.0f);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

/**
 * @brief glfw callback function for when the frame buffer (or window) gets resized
 * @param window pointer to active window
 * @param width new framebuffer width
 * @param height new framebuffer height
 */
void frameBufferResize(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/**
 * @brief glfw callback on mouse scroll
 * @param window pointer to active window
 * @param deltaX the scroll delta on scroll axis X
 * @param deltaY the scroll delta on scroll axis Y
 */
void onScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	scrollY += deltaY;
}

/**
 * @brief glfw keyCallback on key events
 * @param window pointer to active window
 * @param key the key code of the key that caused the event
 * @param scancode a system and platform specific constant
 * @param action type of key event: GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
 * @param mods modifier keys held down on event
 */
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

/**
* @brief convert angle from degree to radian convention
* @param degree the angle in degree to convert
* @return the angle in radians
*/
float degToRad(float deg)
{
	return deg * glm::pi<float>()/180.0f;
}

