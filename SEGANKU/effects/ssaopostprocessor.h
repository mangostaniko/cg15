#ifndef SSAOPOSTPROCESSOR_H
#define SSAOPOSTPROCESSOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>

#include "../shader.h"

/**
 * @brief The SSAOPostprocessor class facilitates Screen Space Ambient Occlusion
 * using a two pass rendering pipeline, where in the first pass the screen colors
 * and other information are rendered to textures to allow for postprocessing,
 * and in the second pass the postprocessed texture is rendered to a screen filling quad.
 */
class SSAOPostprocessor
{

	GLuint fboColor, screenColorTexture, screenDepthBuffer;
	GLuint fboViewPos, viewPosTexture;
	GLuint fboSSAO, ssaoTexture;

	GLuint screenQuadVAO, screenQuadVBO;

	Shader *ssaoShader = nullptr;
	Shader *blurMixingShader = nullptr;

	const static GLuint RANDOM_VECTOR_ARRAY_SIZE = 128;

	/**
	 * @brief draw a screen filling quad
	 */
	void drawQuad();

public:
	SSAOPostprocessor(int windowWidth, int windowHeight);
	~SSAOPostprocessor();

	/**
	 * @brief bind framebuffer in which screen colors should be stored for ssao postprocessing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindFramebufferColor();

	/**
	 * @brief bind framebuffer in which view space vertex positions should be stored for ssao postprocessing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindFramebufferViewPos();

	/**
	 * @brief apply ssao postprocessing to the prerendered color texture,
	 * switch back to default framebuffer and render the result to a screen filling quad.
	 * this needs certain information rendered to textures via the bindFramebuffer methods.
	 */
	void renderPostprocessedSSAO(const glm::mat4 &projMat, bool blurEnabled);

};

#endif // SSAOPOSTPROCESSOR_H
