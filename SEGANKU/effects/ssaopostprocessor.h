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

	GLuint fboScreenData, screenColorTexture, viewPosTexture, screenDepthBuffer;
	GLuint fboSSAO, ssaoTexture;

	GLuint screenQuadVAO, screenQuadVBO;

	Shader *ssaoShader = nullptr;

	const static GLuint RANDOM_VECTOR_ARRAY_SIZE = 64; // reference uses 64 [increase for better quality]

	/**
	 * @brief draw a screen filling quad
	 */
	void drawQuad();

public:
	SSAOPostprocessor(int windowWidth, int windowHeight, int samples);
	~SSAOPostprocessor();

	/**
	 * @brief bind framebuffer in which screen colors and view space vertex positions should be stored for ssao postprocessing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindScreenDataFramebuffer();

	/**
	 * @brief calulate the resulting ssao factors for each fragment
	 * and store it in a texture attached to the fboSSAO
	 * @param projMat the projection matrix to use in the render pipeline
	 * this needs certain information rendered to textures after binding via the bindScreenDataFramebuffer.
	 */
	void calulateSSAOValues(const glm::mat4 &projMat);

	/**
	 * @brief bind the texture which stores the ssao results after calulateSSAOValues
	 * to given shader locaton and texture unit
	 * @param ssaoTexShaderLocation the shader location
	 * @param textureUnit the texture unit
	 */
	void bindSSAOResultTexture(GLint ssaoTexShaderLocation, GLuint textureUnit);
};

#endif // SSAOPOSTPROCESSOR_H