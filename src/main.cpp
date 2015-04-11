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

GLFWwindow *window;

Camera *camera;
Shader *shader;
Texture *texture;
std::vector<std::shared_ptr<SceneObject>> cubes;

void frameBufferResize(GLFWwindow *window, int width, int height);


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

	glfwSetFramebufferSizeCallback(window, frameBufferResize);


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
		if (deltaShowFpsTime > 0.1) {
			deltaShowFpsTime = 0;
			std::cout << "fps: " << (int)(1/deltaT) << std::endl;
		}
		
		handleInput(window, deltaT);

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

	camera = new Camera(glm::mat4(1.0f), glm::pi<float>()/3, width/(float)height, 0.2f, 20.0f); // mat, fov, aspect, znear, zfar
	camera->translate(glm::vec3(0, 0, 6), SceneObject::RIGHT); // move back a bit to see something


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

}

void update(float timeDelta)
{
	camera->update(timeDelta);
	//camera->lookAt(glm::vec3(cubes.at(6)->getLocation())); // broken
	/*
	// camera movement
	float moveSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 20.0f;
	}

    if (glfwGetKey(window, 'W')) {
		camera->translate(camera->getMatrix()[2].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    } else if (glfwGetKey(window, 'S')) {
		camera->translate(camera->getMatrix()[2].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
	}
    if (glfwGetKey(window, 'A')) {
		camera->translate(camera->getMatrix()[0].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    } else if (glfwGetKey(window, 'D')) {
		camera->translate(camera->getMatrix()[0].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
    }
	if (glfwGetKey(window, 'Q')) {
	    camera->translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	} else if (glfwGetKey(window, 'E')) {
	    camera->translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	*/

	// rotate camera based on mouse movement
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::RIGHT);
	camera->rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

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
	for (unsigned i = 0; i < cubes.size(); ++i) {
		cubes[i]->draw();
	}

}

void cleanup()
{
	delete shader; shader = nullptr;
	delete texture; texture = nullptr;
	delete camera; camera = nullptr;
}

void frameBufferResize(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void handleInput(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 20.0f;
	}
	
	//////////////////////////
	/// CAMERA MOVEMENT
	//////////////////////////

	if (glfwGetKey(window, 'W') && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		camera->translate(camera->getMatrix()[2].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'S') && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		camera->translate(camera->getMatrix()[2].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	if (glfwGetKey(window, 'A') && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		camera->translate(camera->getMatrix()[0].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'D') && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		camera->translate(camera->getMatrix()[0].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	if (glfwGetKey(window, 'Q')) {
		camera->translate(glm::vec3(0, 1, 0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
		camera->translate(glm::vec3(0, 1, 0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}


	//////////////////////////
	/// OBJECT MOVEMENT
	//////////////////////////

	if (glfwGetKey(window, GLFW_KEY_W) && !glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		cubes[4]->translate(glm::vec3(0, 0, -1) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, GLFW_KEY_S) && !glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		cubes[4]->translate(glm::vec3(0, 0, 1) * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, GLFW_KEY_A) && !glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		cubes[4]->translate(glm::vec3(-1, 0, 0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, GLFW_KEY_D) && !glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		cubes[4]->translate(glm::vec3(1, 0, 0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
}

