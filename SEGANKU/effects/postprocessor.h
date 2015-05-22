#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>

#include "../shader.h"

/**
 * @brief The PostProcessor class is used to facilitate a two pass rendering pipeline,
 * where in the first pass, the whole screen colors and other information are rendered to textures
 * which allows for postprocessing, and in the second pass the postprocessed texture is
 * rendered to a screen filling quad.
 */
class PostProcessor
{

	GLuint fboColor, screenColorTexture, screenDepthBuffer;
	GLuint fboViewPos, viewPosTexture;

	GLuint screenQuadVAO, screenQuadVBO;

	Shader *postprocessShader = nullptr;

	const static GLuint RANDOM_VECTOR_ARRAY_SIZE = 128;


public:
	PostProcessor(int windowWidth, int windowHeight);
	~PostProcessor();

	/**
	 * @brief bind framebuffer in which screen colors should be stored for post processing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindFramebufferColor();

	/**
	 * @brief bind framebuffer in which view space vertex positions should be stored for post processing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindFramebufferViewPos();

	/**
	 * @brief set shader to do the postprocessing of the prerendered screen texture
	 * and the rendering of the result to a screen filling quad
	 * @param postprocessVertextShaderPath the postprocessing vertex shader
	 * @param postprocessFragmentShaderPath the postprocessing fragment shader
	 */
	void setPostprocessShader(const std::string &postprocessVertextShaderPath,
	                          const std::string &postprocessFragmentShaderPath);

	/**
	 * @brief apply postprocessing to the prerendered color texture,
	 * switch back to default framebuffer and render the result to a screen filling quad.
	 * this needs certain information rendered to textures via the bindFramebuffer methods.
	 */
	void renderPostprocessedSSAO(const glm::mat4 &projMat);

};

#endif // POSTPROCESSOR_H
