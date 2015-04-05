#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <sstream>


// glsl shader sources
const GLchar* vertexSource =
"#version 330 core \n"
"in vec3 position;"
"in vec3 color;"
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
"out vec4 out_color;"
"void main() {"
"   out_color = vec4(1 - color_varying, 1.0);"
//"out_color = vec4(texture(colorTexture, texCoord).rgb) * 0.5"
"}";

void init(GLFWwindow *window);
void update(float time_delta);
void draw();
void cleanup();

/*
Shader *shader;
Texture *texture;
Cube *cube;
*/

void frameBufferResize(GLFWwindow *window, int width, int height);

int main(int argc, char **argv)
{
	/* HANDLE COMMAND LINE PARAMETERS */

	int windowWidth = 800;
	int windowHeigth = 600;
	int refresh_rate = 60;
	bool fullscreen = 0;

	if (argc == 1) {
		// no parameters specified, continue with default values
		
	} else if (argc != 4 || (std::stringstream(argv[1]) >> windowWidth).fail() || (std::stringstream(argv[2]) >> windowHeigth).fail() || (std::stringstream(argv[3]) >> fullscreen).fail()) {
		// if parameters are specified, must conform to given format
		
		std::cout << "USAGE: <resolution width> <resolution height> <fullscreen? 0/1>\n";
		return -1;
	}

	/* INIT WINDOW AND OPENGL CONTEXT */

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);

	GLFWmonitor *monitor = nullptr;
	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}

	GLFWwindow *window = nullptr;
	window = glfwCreateWindow(windowWidth, windowHeigth, "OpenGL GLFW Test", monitor, NULL);
	if (!window)
	{
		std::cerr << "Failed to open GLFW window.\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// print OpenGL version
	std::cerr << "OpenGL " << glGetString(GL_VERSION) << std::endl;
	std::cerr << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << glewGetErrorString(err);
	}

	glfwSetFramebufferSizeCallback(window, frameBufferResize);


	/* INIT MATRICES */

	//auto proj = glm::perspective(60.0f, width / (float)height, 0.2f, 20.0f);
	//auto view = glm::


	/* INIT VERTEX DATA */

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
		 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	// create and init buffer from vertex data. the buffer lies on the gpu!
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	/* INIT SHADERS */

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
	glBindFragDataLocation(shaderProgram, 0, "out_color"); // binds resulting fragment color
	glLinkProgram(shaderProgram); // out variables from vs bound to in variables in fs
	glUseProgram(shaderProgram); // only one can be active at a time


	/* get references to shader uniforms
	* note: uniforms are so named because they do not change from one execution
	* of a shader program to the next within a particular rendering call
	* i.e. same for all vertices/fragments */

	GLint uniDeltaPos = glGetUniformLocation(shaderProgram, "uni_delta_pos");


	//////////////////////////
	/// MAIN LOOP
	//////////////////////////

	float time = 0.0;
	float deltaT = 0.0;
	float lastTime = 0.0;

	while (!glfwWindowShouldClose(window)) {

		//////////////////////////
		/// HANDLE EVENTS
		//////////////////////////

		time = glfwGetTime();
		deltaT = time - lastTime;
		lastTime = time;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		//////////////////////////
		/// UPDATE
		//////////////////////////

		update(deltaT);

		// rotate triangle using delta pos 
		glUniform3f(uniDeltaPos, glm::sin(time * 10.0f) / 4.0f, glm::cos(time * 10.0f) / 4.0f, 0.0f);


		//////////////////////////
		/// DRAW
		//////////////////////////

		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// specify the format of the attributes used in the shaders
		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		glEnableVertexAttribArray(posAttrib); // enable vertex attribute of given index
		glVertexAttribPointer(
			posAttrib,     // index of vertex attribute to be modified (depends on shader).
			3,                // number of elements of the attrib (three for x, y, z)
			GL_FLOAT,         // type
			GL_FALSE,         // normalized?
			6 * sizeof(float),  // size of whole vertex attrib array
			(void*)0          // offset of this vertex attrib within the array
			);

		GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, 3); // draw 3 vertices (not triangles!) starting from index 0, in GL_TRIANGLES draw mode

		// end the current frame (swaps the front and back buffers)
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// release resources

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	glfwTerminate();
	return 0;
}



void init(GLFWwindow *window)
{

}

void update(float time_delta)
{

}

void draw()
{

}

void cleanup()
{
	//delete cube; cube = nullptr;
	//delete shader; shader = nullptr;
	//delete texture; texture = nullptr;
}

void frameBufferResize(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

