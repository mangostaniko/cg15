#include "ssaopostprocessor.h"

// vertex positions and uvs defining a quad. used to render the screen texture.
static const GLfloat quadVertices[] = {
    // positions   // uvs
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

SSAOPostprocessor::SSAOPostprocessor(int windowWidth, int windowHeight, int samples)
{
	////////////////////////////////////
	/// SETUP SCREEN FILLING QUAD
    /// setup vao for the screen quad onto which the screen texture will be mapped.
	////////////////////////////////////

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	GLint positionAttribIndex   = 0;
	GLint uvsAttribIndex		= 1;
    glEnableVertexAttribArray(positionAttribIndex);
	glEnableVertexAttribArray(uvsAttribIndex);
    glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(uvsAttribIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);


	////////////////////////////////////
	/// SETUP FRAMEBUFFERS
	/// setup framebuffers with texture or renderbuffer objects as attachments
	/// - texture: optimized for later sampling (e.g. for postprocessing)
	/// - renderbuffer object: optimized for use as render target, less flexible
    ////////////////////////////////////

	setupFramebuffers(windowWidth, windowHeight);


	////////////////////////////////////
	/// SETUP SSAO SHADER
    /// compile shader programs and pass random vectors to ssao shader used for depth sampling.
    /// other uniforms are passed to ssao shader each frame and are not included here.
    ////////////////////////////////////

	ssaoShader = new Shader("../SEGANKU/shaders/ssao.vert", "../SEGANKU/shaders/ssao.frag");

	// create array of random vectors for depth sampling in ssao shader
	glm::vec3 randomVectors[RANDOM_VECTOR_ARRAY_SIZE];
    for (GLuint i = 0; i < RANDOM_VECTOR_ARRAY_SIZE; ++i) {

        glm::vec3 randomVector;
        randomVector.x = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.y = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.z = 2.0f * (float)rand()/RAND_MAX - 1.0f;

        // scale vectors depending on array index
		// quadratic falloff so that more points lie closer to the origin
		float scale = i / (float)(RANDOM_VECTOR_ARRAY_SIZE);
        randomVector *= (0.4 + 0.6f * scale * scale);

        randomVectors[i] = randomVector;
		//std::cout << "x: " << randomVector.x << ", y: " << randomVector.y << ", z: " << randomVector.z << std::endl;
    }

	// use uniform buffer object to pass random vectors to ssao shader for better performance
	glUniformBlockBinding(ssaoShader->programHandle, glGetUniformBlockIndex(ssaoShader->programHandle, "RandomVectors"), 0);
	GLuint uboRandomVectors;
	glGenBuffers(1, &uboRandomVectors);
	glBindBuffer(GL_UNIFORM_BUFFER, uboRandomVectors);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) * RANDOM_VECTOR_ARRAY_SIZE, &randomVectors[0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboRandomVectors, 0, sizeof(glm::vec3) * RANDOM_VECTOR_ARRAY_SIZE);
	//alternative using simple uniform array:
    //glUniform3fv(glGetUniformLocation(postprocessShader->programHandle, "randomVectors"), RANDOM_VECTOR_ARRAY_SIZE, (const GLfloat*)&randomVectors[0]);

}

SSAOPostprocessor::~SSAOPostprocessor()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboScreenData);
	glDeleteTextures(1, &screenColorTexture);
	glDeleteTextures(1, &viewPosTexture);
	glDeleteRenderbuffers(1, &screenDepthBuffer);

	glDeleteFramebuffers(1, &fboSSAO);
	glDeleteTextures(1, &ssaoTexture);

	glDeleteBuffers(1, &screenQuadVBO);
	glDeleteVertexArrays(1, &screenQuadVAO);

	delete ssaoShader; ssaoShader = nullptr;

}

void SSAOPostprocessor::setupFramebuffers(int windowWidth, int windowHeight)
{

	glDeleteFramebuffers(1, &fboScreenData);
	glDeleteTextures(1, &screenColorTexture);
	glDeleteTextures(1, &viewPosTexture);
	glDeleteRenderbuffers(1, &screenDepthBuffer);

	glDeleteFramebuffers(1, &fboSSAO);
	glDeleteTextures(1, &ssaoTexture);


	// generate screen color texture
	// note: GL_NEAREST interpolation is ok since there is no subpixel sampling anyway
	glGenTextures(1, &screenColorTexture);
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// generate vertex view space position texture
	glGenTextures(1, &viewPosTexture);
	glBindTexture(GL_TEXTURE_2D, viewPosTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_BGR, GL_FLOAT, NULL);

	// generate depth renderbuffer. without this, depth testing wont work.
	// we use a renderbuffer since we wont have to sample this, opengl uses it directly.
	glGenRenderbuffers(1, &screenDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, screenDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);

	// generate framebuffer to attach color texture + view space positions texture + depth renderbuffer
	glGenFramebuffers(1, &fboScreenData); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData); // bind fbo to active framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, viewPosTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOPostprocessor: ScreenData Framebuffer not complete" << std::endl;
	}


	// generate ssao texture
	glGenTextures(1, &ssaoTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);

	// generate framebuffer to attach ssao texture
	glGenFramebuffers(1, &fboSSAO); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO); // bind fbo to active framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOPostprocessor: SSAO Framebuffer not complete" << std::endl;
	}


	// bind back to default framebuffer (as created by glfw)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SSAOPostprocessor::bindScreenDataFramebuffer()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 }; // shader output locations
	glDrawBuffers(2, buffers);
}

void SSAOPostprocessor::calulateSSAOValues(const glm::mat4 &projMat)
{
	ssaoShader->useShader();

	glUniformMatrix4fv(glGetUniformLocation(ssaoShader->programHandle, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));

	GLint sampleCountLocation = glGetUniformLocation(ssaoShader->programHandle, "random_vector_array_size");
	glUniform1i(sampleCountLocation, RANDOM_VECTOR_ARRAY_SIZE);

	GLint viewPosTexLocation = glGetUniformLocation(ssaoShader->programHandle, "viewPosTexture");
	glBindFramebuffer(GL_FRAMEBUFFER, fboScreenData);
	glUniform1i(viewPosTexLocation, 4); // bind shader location to texture unit 4
	glActiveTexture(GL_TEXTURE0 + 4); // activate texture unit 4
	glBindTexture(GL_TEXTURE_2D, viewPosTexture); // bind texture to active texture unit

	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOPostprocessor::bindSSAOResultTexture(GLint ssaoTexShaderLocation, GLuint textureUnit)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
	glUniform1i(ssaoTexShaderLocation, textureUnit);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOPostprocessor::drawQuad()
{
	glDisable(GL_DEPTH_TEST); // no need for depth testing since we just draw a single quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST); // reenable depth testing
}
