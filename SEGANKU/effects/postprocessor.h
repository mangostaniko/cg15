#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include "../shader.h"

/**
 * @brief The PostProcessor class is used to implement a two pass rendering pipeline,
 * where in the first pass, the whole screen colors and depth are rendered to textures
 * which allows for postprocessing, and in the second pass the postprocessed texture is
 * rendered to a screen filling quad.
 */
class PostProcessor
{

	GLuint fbo;
	GLuint screenColorTexture, screenDepthTexture;

	GLuint screenQuadVAO, screenQuadVBO;

	Shader *postprocessShader = nullptr;


public:
	PostProcessor(int windowWidth, int windowHeight);
	~PostProcessor();

	/**
	 * @brief bind an alternative framebuffer so that the following render calls
	 * render the screen color and depth to textures, which can be used for post processing.
	 * this should be followed by a call to renderPostprocessed.
	 */
	void bindRenderScreenToTexture();

	/**
	 * @brief set shader to do the postprocessing of the prerendered screen texture
	 * and the rendering of the result to a screen filling quad
	 * @param postprocessVertextShaderPath the postprocessing vertex shader
	 * @param postprocessFragmentShaderPath the postprocessing fragment shader
	 */
	void setPostprocessShader(const std::string &postprocessVertextShaderPath,
	                          const std::string &postprocessFragmentShaderPath);

	/**
	 * @brief apply postprocessing to the prerendered screen texture,
	 * switch back to default framebuffer and render the result to a screen filling quad.
	 * this should be preceded by a call to bindRenderScreenToTexture.
	 */
	void renderPostprocessed();

};

#endif // POSTPROCESSOR_H
