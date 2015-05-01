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
#include <random>
#include <ctime>

#include "shader.h"
#include "sceneobject.h"
#include "camera.h"
#include "player.h"
#include "light.h"


void init(GLFWwindow *window);
void update(float timeDelta);
void draw();
void cleanup();

GLFWwindow *window;
bool running = true;
bool paused = false;

Shader *textureShader, *normalsShader;
Geometry *player;
Geometry *hawk;
Geometry *world;
Camera *camera;
Light *sun;

Geometry *carrot;
glm::vec3 carrotPos;
//std::vector<std::shared_ptr<Geometry>> carrots;
const float timeToStarvation = 60;

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
	//glClearColor(0.68f, 0.78f, 1.0f, 1.0f); // warm sky blue
	//glClearColor(0.11f, 0.1f, 0.14f, 1.0f); // dark grey
	glClearColor(0.f, 0.f, 0.f, 1.f);
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

	init(window);


	//////////////////////////
	/// MAIN LOOP
	//////////////////////////

	double time = 0.0;
	double lastTime = 0.0;
	double deltaT = 0.0;
	double deltaShowFpsTime = 0.0;

	while (running && !glfwWindowShouldClose(window)) {

		glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);

		if (!paused) {

			//////////////////////////
			/// UPDATE
			//////////////////////////

			time = glfwGetTime(); // seconds
			deltaT = time - lastTime;
			lastTime = time;

			// print framerate around every second (console output is costly)
			deltaShowFpsTime += deltaT;
			if (deltaShowFpsTime > 1.0) {
				deltaShowFpsTime = 0;
				std::cout << "fps: " << (int)(1/deltaT) << std::endl;

				// print time until starvation
				std::cout << "time until starvation: " << int(timeToStarvation - glfwGetTime()) << std::endl;
				std::cout << "Location of the sun; " << sun->getLocation().x << " " << sun->getLocation().y << " " << sun->getLocation().z << std::endl;
				std::cout << "Color of the sun; " << sun->getColor().x << " " << sun->getColor().y << " " << sun->getColor().z << std::endl;

			}

			update(deltaT);

			// pause on starvation
			if (glfwGetTime() > timeToStarvation-1) {
				std::cout << "YOU STARVED!!! :(\nPress ESC to exit." << std::endl;
				player->rotateZ(3.14159/2, SceneObject::RIGHT);
				player->translate(glm::vec3(0, 0.3, 0), SceneObject::LEFT);
				paused = true;
			}

		}


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

		if (running) {
			running = !glfwGetKey(window, GLFW_KEY_ESCAPE);
		}

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


	// INIT SHADERS

	textureShader = new Shader("../SEGANKU/shaders/texture_shader.vert", "../SEGANKU/shaders/texture_shader.frag");
	normalsShader = new Shader("../SEGANKU/shaders/normals_shader.vert", "../SEGANKU/shaders/normals_shader.frag");
	textureShader->useShader(); // non-trivial cost


	// INIT WORLD + OBJECTS

	world = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "../data/models/world/world.dae");

	std::default_random_engine randGen(time(nullptr));
	std::uniform_int_distribution<int> randDistribution(-49,49);
	randDistribution(randGen);
	carrotPos = glm::vec3(randDistribution(randGen), 0, randDistribution(randGen));
	carrot = new Geometry(glm::translate(glm::mat4(1.0f), carrotPos), "../data/models/world/carrot.dae");
	
	//const glm::mat4 &modelMatrix_, glm::vec3 endPos, glm::vec3 startCol, glm::vec3 endCol, float seconds

	glm::mat4 lightStart(glm::translate(glm::mat4(1.0f), glm::vec3(-120, 30, 0)));
	sun = new Light(lightStart, glm::vec3(40, 30, 0), glm::vec3(1.f, 0.89f, 0.6f), glm::vec3(0.87f, 0.53f, 0.f), 30.f);


	// INIT PLAYER + CAMERA

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	camera = new Camera(glm::mat4(1.0f), glm::radians(80.0f), width/(float)height, 0.2f, 200.0f); // mat, fov, aspect, znear, zfar

	player = new Player(glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)), camera, window, "../data/models/skunk/skunk.dae");


	// INIT HAWK

	hawk = new Geometry(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(3, 3, 3)), glm::vec3(0, 10, -15)), "../data/models/hawk/hawk.dae");
	hawk->rotateY(glm::radians(270.0), SceneObject::RIGHT);

}

void update(float timeDelta)
{
	player->update(timeDelta);
	if (glm::length(glm::abs(player->getLocation() - carrotPos)) < 1.0f) {
		std::cout << "YOU FOUND THE CARROT!!! WOOHOO!!" << std::endl;
		paused = true;
	}

	//hawk->update(timeDelta);
	hawk->rotateY(3*timeDelta, SceneObject::LEFT);
	hawk->translate(glm::vec3(0, glm::cos(glfwGetTime())/20, 0), SceneObject::RIGHT);
	hawk->rotateZ(glm::cos(glfwGetTime())/200, SceneObject::RIGHT);
	hawk->rotateX(glm::cos(glfwGetTime())/200, SceneObject::RIGHT);

	sun->update(timeDelta);

	// SET POSITION AND COLOR IN SHADER
	GLint lightPosLocation = glGetUniformLocation(textureShader->programHandle, "lightPos");
	glUniform3f(lightPosLocation, sun->getLocation().x, sun->getLocation().y, sun->getLocation().z);

	GLint lightColorLocation = glGetUniformLocation(textureShader->programHandle, "lightColor");
	glUniform3f(lightColorLocation, sun->getColor().x, sun->getColor().y, sun->getColor().z);

}

void draw()
{
	player->draw(textureShader);
	hawk->draw(textureShader);

	world->draw(textureShader);
	carrot->draw(textureShader);

}

void cleanup()
{
	delete textureShader; textureShader = nullptr;
	delete normalsShader; normalsShader = nullptr;
	delete player; player = nullptr;
	delete hawk; hawk = nullptr;
	delete world; world = nullptr;
	delete carrot; carrot = nullptr;
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
