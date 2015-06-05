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
#include "textrenderer.h"
#include "effects/ssaopostprocessor.h"
#include "effects/particlesystem.h"


void init(GLFWwindow *window);
void update(float timeDelta);
void setActiveShader(Shader *shader);
void drawScene();
void drawText(double deltaT);
void cleanup();
void newGame();

GLFWwindow *window;
bool running			= true;
bool paused				= false;
bool foundCarrot		= false;
bool debugInfoEnabled   = true;
bool wireframeEnabled   = false;
bool ssaoEnabled		= false;
bool renderShadowMap	= false;

Shader *textureShader, *ssaoViewPosPrepassShader, *depthMapShader, *debugDepthShader;
Shader *activeShader;
TextRenderer *textRenderer;
ParticleSystem *particleSystem;
SSAOPostprocessor *ssaoPostprocessor;

Player *player; glm::mat4 playerInitTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)));
Geometry *hawk; glm::mat4 hawkInitTransform(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(3, 3, 3)), glm::vec3(0, 10, -15)));
Geometry *world;
Camera *camera;
Light *sun;
const glm::vec3 LIGHT_START(glm::vec3(-120, 200, 0));
const glm::vec3 LIGHT_END(glm::vec3(40, 200, 0));

Geometry *carrot;
glm::vec3 carrotPos;
//std::vector<std::shared_ptr<Geometry>> carrots;
const float timeToStarvation = 60;

// Shadow Map FBO and depth texture
GLuint depthMapFBO;
GLuint depthMap;
const int SM_WIDTH = 4096, SM_HEIGHT = 4096;

void frameBufferResize(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

// ONLY FOR SEBAS DEBUGGING

GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions         // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
// SEBAS DEBUGGING END


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
	glfwSetKeyCallback(window, keyCallback);

	init(window);


	//////////////////////////
	/// MAIN LOOP
	//////////////////////////

	double time = 0.0;
	double lastTime = 0.0;
	double deltaT = 0.0;

	while (running && !glfwWindowShouldClose(window)) {

		if (glfwGetTime() < lastTime) {
			lastTime = 0;
		}
		time = glfwGetTime(); // seconds
		deltaT = time - lastTime;
		lastTime = time;

		// glUseProgram calls are rather expensive state changes, so try to keep to a minimum
		// if more shaders are used for different objects, restructuring of these calls will be necessary
		// since a lot of other calls depend on the currently bound shader
		setActiveShader(textureShader);

		if (!paused) {

			//////////////////////////
			/// UPDATE
			//////////////////////////

			update(deltaT);

			// pause on starvation
			if (glfwGetTime() > timeToStarvation-1) {
				player->rotateZ(3.14159/2, SceneObject::RIGHT);
				player->translate(glm::vec3(0, 0.3, 0), SceneObject::LEFT);
				paused = true;
			}
		}


		//////////////////////////
		/// DRAW 
		//////////////////////////

		/* 
		 * SHADOW MAPPING first pass -> render depth to texture
		 */

		// set viewport and bind framebuffer
		glViewport(0, 0, SM_WIDTH, SM_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// calculate lights projection and view Matrix
		GLfloat nearPlane = 50.f, farPlane = 400.f;
		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
		//glm::mat4 lightProjection = glm::perspective(100.f, (GLfloat) SM_WIDTH / (GLfloat) SM_HEIGHT, nearPlane, farPlane);
		glm::mat4 lightView = glm::lookAt(sun->getLocation(), glm::vec3(0.f), glm::vec3(0, 1, 0));
		glm::mat4 lightVP = lightProjection * lightView;

		// prepare shader
		Shader *lastShader = activeShader;
		setActiveShader(depthMapShader);

		GLint lightVPLocation = glGetUniformLocation(activeShader->programHandle, "lightVP");
		glUniformMatrix4fv(lightVPLocation, 1, GL_FALSE, glm::value_ptr(lightVP));

		// render
		drawScene();

		// bind default FB and reset viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeigth);
		setActiveShader(lastShader); lastShader = nullptr;

		if (ssaoEnabled) {

			ssaoPostprocessor->bindFramebufferColor();
			glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			GLint booleanSMLocation = glGetUniformLocation(activeShader->programHandle, "useShadows");
			glUniform1i(booleanSMLocation, 0);
			drawScene();

			ssaoPostprocessor->bindFramebufferViewPos();
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			lastShader = activeShader;
			setActiveShader(ssaoViewPosPrepassShader);
			glUniformMatrix4fv(glGetUniformLocation(activeShader->programHandle, "viewMat"), 1, GL_FALSE, glm::value_ptr(player->getViewMat()));
			drawScene();

			glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
			ssaoPostprocessor->renderPostprocessedSSAO(player->getProjMat(), true); // bool toggles blur

			setActiveShader(lastShader); lastShader = nullptr;
		}
		else { // no postprocessing but shadow mapping
			glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			GLint booleanSMLocation = glGetUniformLocation(activeShader->programHandle, "useShadows");
			glUniform1i(booleanSMLocation, 1);

			lightVPLocation = glGetUniformLocation(activeShader->programHandle, "lightVP");
			glUniformMatrix4fv(lightVPLocation, 1, GL_FALSE, glm::value_ptr(lightVP));
			
			GLint shadowMapLocation = glGetUniformLocation(activeShader->programHandle, "shadowMap");
			glUniform1i(shadowMapLocation, 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
	
			drawScene();
		}
		
		if (renderShadowMap) {

			lastShader = activeShader;
			setActiveShader(debugDepthShader);

			glUniform1f(glGetUniformLocation(debugDepthShader->programHandle, "near_plane"), nearPlane);
			glUniform1f(glGetUniformLocation(debugDepthShader->programHandle, "far_plane"), farPlane);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			RenderQuad();
			setActiveShader(lastShader); lastShader = nullptr;
		}

		particleSystem->draw(player->getViewMat(), player->getProjMat());

		drawText(deltaT);

		// end the current frame (swaps the front and back buffers)
		glfwSwapBuffers(window);		


		//////////////////////////
		/// ERRORS AND EVENTS
		//////////////////////////

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR) {
			// handle errors
			std::cerr << "ERROR: OpenGL Error " << glErr << std::endl;
		}

		glfwPollEvents();

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
	glfwSetTime(0);

	// enable z buffer test
	glEnable(GL_DEPTH_TEST);

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	paused = false;
	foundCarrot = false;


	// INIT SHADOW MAPPING (Framebuffer + ShadowMap + Shaders)

	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);

	// ShadowMap
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SM_WIDTH, SM_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// SM Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// SM Shaders
	depthMapShader = new Shader("../SEGANKU/shaders/depth_shader.vert", "../SEGANKU/shaders/depth_shader.frag");
	debugDepthShader = new Shader("../SEGANKU/shaders/quad_debug.vert", "../SEGANKU/shaders/quad_debug.frag");


	// INIT TEXT RENDERER
	textRenderer = new TextRenderer("../data/fonts/VeraMono.ttf", width, height);

	// INIT PARTICLE SYSTEM
	particleSystem = new ParticleSystem(glm::mat4(1.0f), "../data/models/skunk/smoke.png", 300, 50.f, 10.f, -0.1f);

	// INIT SSAO POST PROCESSOR AND PREPASS SHADERS
	ssaoPostprocessor = new SSAOPostprocessor(width, height);
	ssaoViewPosPrepassShader = new Shader("../SEGANKU/shaders/view_positions.vert", "../SEGANKU/shaders/view_positions.frag");

	// INIT SHADERS
	textureShader = new Shader("../SEGANKU/shaders/textured_blinnphong.vert", "../SEGANKU/shaders/textured_blinnphong.frag");
	setActiveShader(textureShader); // non-trivial cost
	// note that the following initializations depend are intended to communicate with this shader
	// so dont activate any shader of different structure before those initializations are done


	// INIT WORLD + OBJECTS
	world = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "../data/models/world/world.dae");

	std::default_random_engine randGen(time(nullptr));
	std::uniform_int_distribution<int> randDistribution(-49,49);
	randDistribution(randGen);
	carrotPos = glm::vec3(randDistribution(randGen), 0, randDistribution(randGen));
	carrot = new Geometry(glm::translate(glm::mat4(1.0f), carrotPos), "../data/models/world/carrot.dae");

	//const glm::mat4 &modelMatrix_, glm::vec3 endPos, glm::vec3 startCol, glm::vec3 endCol, float seconds
	sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), LIGHT_END, glm::vec3(1.f, 0.89f, 0.6f), glm::vec3(0.87f, 0.53f, 0.f), timeToStarvation);


	// INIT PLAYER + CAMERA
	camera = new Camera(glm::mat4(1.0f), glm::radians(80.0f), width/(float)height, 0.2f, 200.0f); // mat, fov, aspect, znear, zfar
	player = new Player(playerInitTransform, camera, window, "../data/models/skunk/skunk.dae");

	// INIT HAWK
	hawk = new Geometry(hawkInitTransform, "../data/models/hawk/hawk.dae");
	hawk->rotateY(glm::radians(270.0), SceneObject::RIGHT);

}


void update(float timeDelta)
{
	player->update(timeDelta);
	if (glm::length(glm::abs(player->getLocation() - carrotPos)) < 1.0f) {
		foundCarrot = true;
		paused = true;
	}

	//hawk->update(timeDelta);
	hawk->rotate(0.5f * timeDelta, SceneObject::LEFT, glm::vec3(0.f, 1.f, 0.f));
	//hawk->rotateY(3*timeDelta, SceneObject::LEFT);
	//hawk->translate(glm::vec3(0, glm::cos(glfwGetTime())/20, 0), SceneObject::RIGHT);
	hawk->rotateZ(glm::cos(-glfwGetTime())/2000, SceneObject::RIGHT);
	hawk->rotateX(glm::cos(-glfwGetTime())/2000, SceneObject::RIGHT);

	particleSystem->update(timeDelta, player->getViewMat());

	sun->update(timeDelta);

	// SET POSITION AND COLOR IN SHADER
	GLint lightPosLocation = glGetUniformLocation(activeShader->programHandle, "light.position");
	GLint lightAmbientLocation = glGetUniformLocation(activeShader->programHandle, "light.ambient");
	GLint lightDiffuseLocation = glGetUniformLocation(activeShader->programHandle, "light.diffuse");
	GLint lightSpecularLocation = glGetUniformLocation(activeShader->programHandle, "light.specular");

	GLint materialSpecularLocation = glGetUniformLocation(activeShader->programHandle, "material.specular");
	glUniform3f(materialSpecularLocation, 0.2f, 0.2f, 0.2f);

	glUniform3f(lightPosLocation, sun->getLocation().x, sun->getLocation().y, sun->getLocation().z);
	glUniform3f(lightAmbientLocation, sun->getColor().x * 0.3f, sun->getColor().y * 0.3f, sun->getColor().z * 0.3f);
	glUniform3f(lightDiffuseLocation, sun->getColor().x, sun->getColor().y, sun->getColor().z);
	glUniform3f(lightSpecularLocation, sun->getColor().x * 0.8f, sun->getColor().y * 0.8f, sun->getColor().z * 0.8f);
}


void drawScene()
{
	if (wireframeEnabled) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // enable wireframe

	// DRAW GEOMETRY

	glUniform1f(glGetUniformLocation(activeShader->programHandle, "material.shininess"), 16.f);
	player->draw(activeShader);

	glUniform1f(glGetUniformLocation(activeShader->programHandle, "material.shininess"), 16.f);
	hawk->draw(activeShader);

	glUniform1f(glGetUniformLocation(activeShader->programHandle, "material.shininess"), 32.f);
	world->draw(activeShader);

	glUniform1f(glGetUniformLocation(activeShader->programHandle, "material.shininess"), 2.f);
	carrot->draw(activeShader);

	if (wireframeEnabled) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); // disable wireframe
}


void drawText(double deltaT)
{
	glDisable(GL_DEPTH_TEST);

	if (debugInfoEnabled) {

		textRenderer->renderText("delta time: " + std::to_string(int(deltaT*1000 + 0.5)) + " ms", 25.0f, 25.0f, 0.4f, glm::vec3(1));
		textRenderer->renderText("fps: " + std::to_string(int(1/deltaT + 0.5)), 25.0f, 50.0f, 0.4f, glm::vec3(1));

		if (!paused) {
			// draw time until starvation
			textRenderer->renderText("time until starvation: " + std::to_string(int(timeToStarvation - glfwGetTime())), 25.0f, 100.0f, 0.4f, glm::vec3(1));
		}
	}
	if (paused && !foundCarrot) {
		textRenderer->renderText("YOU STARVED :( TRY LOOKING HARDER NEXT TIME.", 25.0f, 100.0f, 0.5f, glm::vec3(1));
	}
	else if (paused && foundCarrot) {
		textRenderer->renderText("CONGRATULATIONS!!! YOU FOUND THE CARROT!!", 25.0f, 100.0f, 0.5f, glm::vec3(1));
	}

	glEnable(GL_DEPTH_TEST);
}


void newGame()
{
	delete sun;

	glfwSetTime(0);

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	// RESET WORLD + OBJECTS
	std::default_random_engine randGen(time(nullptr));
	std::uniform_int_distribution<int> randDistribution(-49, 49);
	randDistribution(randGen);
	carrotPos = glm::vec3(randDistribution(randGen), 0, randDistribution(randGen));
	carrot->setTransform(glm::translate(glm::mat4(1.0f), carrotPos));

	sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), LIGHT_END, glm::vec3(1.f, 0.89f, 0.6f), glm::vec3(0.87f, 0.53f, 0.f), timeToStarvation);

	// RESET PLAYER + CAMERA
	player->setTransform(playerInitTransform);
	hawk->setTransform(hawkInitTransform);
	hawk->rotateY(glm::radians(270.0), SceneObject::RIGHT);

	paused = false;
	foundCarrot = false;
}


void cleanup()
{
	delete textureShader; textureShader = nullptr;
	delete ssaoViewPosPrepassShader; ssaoViewPosPrepassShader = nullptr;
	activeShader = nullptr;

	delete textRenderer; textRenderer = nullptr;
	delete particleSystem; particleSystem = nullptr;
	delete ssaoPostprocessor; ssaoPostprocessor = nullptr;

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
	    player->toggleNavMode();
	}

	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS){
		newGame();
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		particleSystem->respawn();
	}

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
	    debugInfoEnabled = !debugInfoEnabled;
	}

	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
	    wireframeEnabled = !wireframeEnabled;
	}	

	if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS) {
		ssaoEnabled = !ssaoEnabled;
	}

	if (glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS) {
		renderShadowMap = !renderShadowMap;
	}
}


void setActiveShader(Shader *shader)
{
	activeShader = shader;
	activeShader->useShader();
}
