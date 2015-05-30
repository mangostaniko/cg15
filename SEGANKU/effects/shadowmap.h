#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>

#include "../shader.h"

/**
 * @brief The ShadowMap class.
 */
class ShadowMap
{
	GLuint screenDepthBuffer;
	GLuint shadowMap;

	Shader *depthShader = nullptr;
	Shader *shadowMapShader = nullptr;

	Shader *renderDepthQuadShader = nullptr;

public:
	ShadowMap(int windowWidth, int windowHeight);
	~ShadowMap();

	/**
	 * @brief bind framebuffer in which screen colors should be stored for ssao postprocessing.
	 * after binding this, execute the required draw calls using appropriate shaders.
	 */
	void bindScreenDepthBuffer();
};

#endif // SHADOWMAP_H
