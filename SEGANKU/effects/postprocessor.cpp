#include "postprocessor.h"

// vertex positions and uvs defining a quad. used to render the screen texture.
GLfloat quadVertices[] = {
    // positions   // uvs
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

PostProcessor::PostProcessor(int windowWidth, int windowHeight)
{
	// setup vao for the screen quad onto which the screen texture will be mapped
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


	// setup framebuffers with texture or renderbuffer objects as attachment
	// - texture: optimized for later sampling (e.g. postprocessing)
	// - renderbuffer object: optimized for use as render target, less flexible.
	// here we use two texture attachments: one for the color and one for the depth

	// generate screen color texture
	glGenTextures(1, &screenColorTexture);
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// generate depth renderbuffer. without this, depth testing wont work.
	// we use a renderbuffer since we wont have to sample this, opengl uses it directly.
	glGenRenderbuffers(1, &screenDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, screenDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);

	// generate vertex view space position texture
	glGenTextures(1, &viewPosTexture);
	glBindTexture(GL_TEXTURE_2D, viewPosTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	// generate framebuffers and attach textures / renderbuffers

	glGenFramebuffers(1, &fboColor); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fboColor); // bind fbo to active framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in Postprocessor::Postprocessor: Color Framebuffer not complete" << std::endl;
	}

	glGenFramebuffers(1, &fboViewPos);
	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewPosTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in Postprocessor::Postprocessor: ViewPos Framebuffer not complete" << std::endl;
	}

	// bind back to default framebuffer (as created by glfw)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

PostProcessor::~PostProcessor()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboColor);
	glDeleteTextures(1, &screenColorTexture);
	glDeleteFramebuffers(1, &fboViewPos);
	glDeleteTextures(1, &viewPosTexture);

	glDeleteBuffers(1, &screenQuadVBO);
	glDeleteVertexArrays(1, &screenQuadVAO);

	delete postprocessShader; postprocessShader = nullptr;

}

void PostProcessor::bindFramebufferColor()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fboColor);
}

void PostProcessor::bindFramebufferViewPos()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
}

void PostProcessor::setPostprocessShader(const std::string &postprocessVertextShaderPath,
                                         const std::string &postprocessFragmentShaderPath)
{
	if (postprocessShader) {
		delete postprocessShader; postprocessShader = nullptr;
	}
	postprocessShader = new Shader(postprocessVertextShaderPath, postprocessFragmentShaderPath);
}

void PostProcessor::renderPostprocessedSSAO(const glm::mat4 &projMat)
{

	// setup postprocessing shader

	postprocessShader->useShader();

	GLint screenColorTexLocation = glGetUniformLocation(postprocessShader->programHandle, "screenColorTexture");
	GLint viewPosTexLocation = glGetUniformLocation(postprocessShader->programHandle, "viewPosTexture");

	glBindFramebuffer(GL_FRAMEBUFFER, fboColor);
	glUniform1i(screenColorTexLocation, 0); // bind shader location to texture unit 0
	glActiveTexture(GL_TEXTURE0 + 0); // activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, screenColorTexture); // bind texture to active texture unit

	glBindFramebuffer(GL_FRAMEBUFFER, fboViewPos);
	glUniform1i(viewPosTexLocation, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, viewPosTexture);

	glUniformMatrix4fv(glGetUniformLocation(postprocessShader->programHandle, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));


	// create array of random vectors for ssao sampling
	glm::vec3 randomVectors[RANDOM_VECTOR_ARRAY_SIZE];
    for (GLuint i = 0; i < RANDOM_VECTOR_ARRAY_SIZE; ++i) {

        float scale = i / (float)(RANDOM_VECTOR_ARRAY_SIZE);
        glm::vec3 randomVector;
        randomVector.x = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.y = 2.0f * (float)rand()/RAND_MAX - 1.0f;
        randomVector.z = 2.0f * (float)rand()/RAND_MAX - 1.0f;

        // use a quadratic falloff so that more points lie closer to the origin
        randomVector *= (0.1f + 0.9f * scale * scale);

        randomVectors[i] = randomVector;
		//std::cout << "x: " << randomVector.x << ", y: " << randomVector.y << ", z: " << randomVector.z << std::endl;
    }


	// use uniform buffer object to pass random vectors to shader for better performance
	glUniformBlockBinding(postprocessShader->programHandle, glGetUniformBlockIndex(postprocessShader->programHandle, "RandomVectors"), 0);
	GLuint uboRandomVectors;
	glGenBuffers(1, &uboRandomVectors);
	glBindBuffer(GL_UNIFORM_BUFFER, uboRandomVectors);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) * RANDOM_VECTOR_ARRAY_SIZE, &randomVectors[0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboRandomVectors, 0, sizeof(glm::vec3) * RANDOM_VECTOR_ARRAY_SIZE);
	//alternative using simple uniform array:
    //glUniform3fv(glGetUniformLocation(postprocessShader->programHandle, "randomVectors"), RANDOM_VECTOR_ARRAY_SIZE, (const GLfloat*)&randomVectors[0]);


	// render quad to default framebuffer using postprocessing shader

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind back to default framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST); // no need for depth testing since we just draw a single quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST); // reenable depth testing

}

