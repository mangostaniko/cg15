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

SSAOPostprocessor::SSAOPostprocessor(int windowWidth, int windowHeight)
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

	// generate screen color texture
	// note: GL_NEAREST interpolation is ok since there is no subpixel sampling anyway
	glGenTextures(1, &screenColorTexture);
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// generate depth renderbuffer. without this, depth testing wont work.
	// we use a renderbuffer since we wont have to sample this, opengl uses it directly.
	glGenRenderbuffers(1, &screenDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, screenDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);

	// generate framebuffer with color texture + depth renderbuffer attachments
	glGenFramebuffers(1, &fboColor); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fboColor); // bind fbo to active framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOPostprocessor: Color Framebuffer not complete" << std::endl;
	}

	// generate vertex view space position texture
	glGenTextures(1, &viewPosTexture);
	glBindTexture(GL_TEXTURE_2D, viewPosTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_BGR, GL_FLOAT, NULL);

	// generate framebuffer with view space positions texture
	glGenFramebuffers(1, &fboViewPos);
	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewPosTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOPostprocessor: ViewPos Framebuffer not complete" << std::endl;
	}

	// generate ssao texture
	glGenTextures(1, &ssaoTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);

	// generate framebuffer with ssao texture
	glGenFramebuffers(1, &fboSSAO); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO); // bind fbo to active framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in SSAOPostprocessor: SSAO Framebuffer not complete" << std::endl;
	}

	// bind back to default framebuffer (as created by glfw)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	////////////////////////////////////
	/// SETUP SSAO AND BLUR SHADERS
    /// compile shader programs and pass random vectors to ssao shader used for depth sampling.
    /// other uniforms are passed to ssao shader each frame and are not included here.
    /// the blurMixing shader is used to blur the ssao result and mix with the screen color
    ////////////////////////////////////

	ssaoShader = new Shader("../SEGANKU/shaders/ssao.vert", "../SEGANKU/shaders/ssao.frag");
	blurMixingShader = new Shader("../SEGANKU/shaders/ssao_blur_mixing.vert", "../SEGANKU/shaders/ssao_blur_mixing.frag");

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
        randomVector *= (0.1f + 0.9f * scale * scale);

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

	glDeleteFramebuffers(1, &fboColor);
	glDeleteTextures(1, &screenColorTexture);
	glDeleteRenderbuffers(1, &screenDepthBuffer);

	glDeleteFramebuffers(1, &fboViewPos);
	glDeleteTextures(1, &viewPosTexture);

	glDeleteFramebuffers(1, &fboSSAO);
	glDeleteTextures(1, &ssaoTexture);

	glDeleteBuffers(1, &screenQuadVBO);
	glDeleteVertexArrays(1, &screenQuadVAO);

	delete ssaoShader; ssaoShader = nullptr;
	delete blurMixingShader; blurMixingShader = nullptr;

}

void SSAOPostprocessor::bindFramebufferColor()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fboColor);
}

void SSAOPostprocessor::bindFramebufferViewPos()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
}

void SSAOPostprocessor::renderPostprocessedSSAO(const glm::mat4 &projMat, bool blurEnabled)
{

	// render quad to ssao framebuffer using ssao shader

	ssaoShader->useShader();

	glUniformMatrix4fv(glGetUniformLocation(ssaoShader->programHandle, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));

	GLint viewPosTexLocation = glGetUniformLocation(ssaoShader->programHandle, "viewPosTexture");
	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
	glUniform1i(viewPosTexLocation, 0); // bind shader location to texture unit 0
	glActiveTexture(GL_TEXTURE0 + 0); // activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, viewPosTexture); // bind texture to active texture unit

	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawQuad();


	// render quad to default framebuffer using ssao blur and mixing shader

	blurMixingShader->useShader();

	GLint blurEnabledLocation = glGetUniformLocation(blurMixingShader->programHandle, "blurEnabled");
	glUniform1f(blurEnabledLocation, (blurEnabled ? 1.0f : 0.0f));

	GLint ssaoTexLocation = glGetUniformLocation(blurMixingShader->programHandle, "ssaoTexture");
	glBindFramebuffer(GL_FRAMEBUFFER, fboSSAO);
	glUniform1i(ssaoTexLocation, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);

	GLint screenColorTexLocation = glGetUniformLocation(blurMixingShader->programHandle, "screenColorTexture");
	glBindFramebuffer(GL_FRAMEBUFFER, fboColor);
	glUniform1i(screenColorTexLocation, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind back to default framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawQuad();

}

void SSAOPostprocessor::drawQuad()
{
	glDisable(GL_DEPTH_TEST); // no need for depth testing since we just draw a single quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST); // reenable depth testing
}
