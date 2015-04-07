#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <sstream>

#include "shader.h"
#include "sceneobject.h"
#include "sceneobjects/cube.h"


// glsl shader sources
const GLchar* vertexSource =
"#version 330 core \n"
"layout(location=0) in vec3 position;"
"layout(location=1) in vec3 color;"
//"in vec2 uv;"
//"out vec2 texCoord;"
"out vec3 color_varying;"
"uniform vec3 uni_delta_pos;"
"void main() {"
"   color_varying = color;"
"   gl_Position = vec4(position + uni_delta_pos, 1.0);"
"}";
// out variables are interpolated and passed to fragment shader in variables
const GLchar* fragmentSource =
"#version 330 core \n"
"in vec3 color_varying;"
//"in vec2 texCoord;"
"layout(location=0) out vec4 out_color;"
"void main() {"
"   out_color = vec4(1 - color_varying, 1.0);"
//"out_color = vec4(texture(colorTexture, texCoord).rgb) * 0.5"
"}";

void init(GLFWwindow *window);
void update(float timeDelta);
void draw();
void cleanup();

Shader *shader;
SceneObject *testObject;
// Texture *texture;

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

	GLFWwindow *window = nullptr;
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

	glClearColor(0.11f, 0.1f, 0.14f, 1.0f);
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

///
	init(window);


	// INIT MATRICES


/*/

	// INIT VERTEX DATA

	GLuint vao; // stores id by which vertex array object is accessed (defines structure of vertex data).
	glGenVertexArrays(1, &vao); // generate name (ID) to access vertex array
	glBindVertexArray(vao); // bind struct to context such that other functions will operate on it

	GLuint vbo; // stores id by which vertex buffer object is accessed (actual vertex data).
	glGenBuffers(1, &vbo); // generate name
	glBindBuffer(GL_ARRAY_BUFFER, vbo); // bind vbo to the current array buffer context

	static const GLfloat vertices[] = {
	//  Position            Color
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	     0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 1.0f
	};

	static const GLuint indices[] = {
		0, 1, 2,
	    0, 2, 3,
	};


	// create and init buffer from vertex data. the buffer lies on the gpu!
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	// INIT SHADERS

	// compile glsl vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // generate shader name / handle
	glShaderSource(vertexShader, 1, &vertexSource, NULL); // load shader from char array
	glCompileShader(vertexShader);

	char vertex_shader_infolog[512];
	glGetShaderInfoLog(vertexShader, 512, NULL, vertex_shader_infolog);
	if (vertex_shader_infolog[0]) std::cout << vertex_shader_infolog << "\n";

	// compile glsl fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // generate shader name / handle
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL); // load shader from char array
	glCompileShader(fragmentShader);

	char fragment_shader_infolog[512];
	glGetShaderInfoLog(fragmentShader, 512, NULL, fragment_shader_infolog);
	if (vertex_shader_infolog[0]) std::cout << fragment_shader_infolog << "\n";

	// combine shaders into a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram); // out variables from vs bound to in variables in fs
	glUseProgram(shaderProgram); // only one can be active at a time

	// get references to shader uniforms
	// note: uniforms are so named because they do not change from one execution
	// of a shader program to the next within a particular rendering call
	// i.e. same for all vertices/fragments

	GLint uniDeltaPos = glGetUniformLocation(shaderProgram, "uni_delta_pos");

//*/


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

		//////////////////////////
		/// UPDATE
		//////////////////////////
///
		update(deltaT);
/*/
		// rotate triangle using delta pos 
		glUniform3f(uniDeltaPos, glm::sin(time * 10.0f) / 4.0f, glm::cos(time * 10.0f) / 4.0f, 0.0f);
//*/

		//////////////////////////
		/// DRAW
		//////////////////////////

		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

///
		draw();
/*/
		// specify the format of the attributes used in the shaders
		GLint posAttrib = 0;
		glEnableVertexAttribArray(posAttrib); // enable vertex attribute of given index
		glVertexAttribPointer(
			posAttrib,          // index of vertex attribute to be modified (depends on shader).
			3,                  // number of elements of the attrib (three for x, y, z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			6 * sizeof(float),  // size of whole vertex attrib array
			(void*)0            // offset of this vertex attrib within the array
			);

		GLint colAttrib = 1;
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, indices); // draw vertices (not triangles!) starting from index 0, in GL_TRIANGLES draw mode

//*/

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

/*//
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
//*/

	glfwTerminate();
	exit(EXIT_SUCCESS); // for system independent success code
	return 0; // to silence compiler warnings
}



void init(GLFWwindow *window)
{
	shader = new Shader("../src/shaders/testshader.vert", "../src/shaders/testshader.frag");
	shader->useShader();
	testObject = new Cube(glm::mat4(1.0f), shader);

}

void update(float timeDelta)
{
	testObject->update();
}

void draw()
{
	testObject->draw();
}

void cleanup()
{
	delete shader; shader = nullptr;
	delete testObject; testObject = nullptr;
	//delete texture; texture = nullptr;
}

void frameBufferResize(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

