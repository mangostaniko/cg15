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
	glGenFramebuffers(1, &fbo); // generate framebuffer object layout in vram and associate handle
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); // bind fbo to active framebuffer

	// to make framebuffer complete, use texture or renderbuffer object as attachment
	// - texture: optimized for later sampling (e.g. postprocessing)
	// - renderbuffer object: optimized for use as render target, less flexible.
	// here we use two texture attachments: one for the color and one for the depth

	// generate screen color texture and attach to framebuffer
	glGenTextures(1, &screenColorTexture);
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);

	// generate screen depth texture and attach to framebuffer
	glGenTextures(1, &screenDepthTexture);
	glBindTexture(GL_TEXTURE_2D, screenDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepthTexture, 0);

	// check framebuffer
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR in Postprocessor::Postprocessor: Framebuffer not complete, status: " << status << std::endl;
	}


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


	// bind back to default framebuffer (as created by glfw)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PostProcessor::~PostProcessor()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteBuffers(1, &screenQuadVBO);
	glDeleteVertexArrays(1, &screenQuadVAO);

	glDeleteTextures(1, &screenColorTexture);
	glDeleteTextures(1, &screenDepthTexture);

	delete postprocessShader; postprocessShader = nullptr;

}

void PostProcessor::bindRenderScreenToTexture()
{
	// first pass.
	// render the whole screen color and depth to textures.

	glBindFramebuffer(GL_FRAMEBUFFER, fbo); // use framebuffer with textures attached
}

void PostProcessor::setPostprocessShader(const std::string &postprocessVertextShaderPath,
                                         const std::string &postprocessFragmentShaderPath)
{
	if (postprocessShader) { delete postprocessShader; postprocessShader = nullptr; }
	postprocessShader = new Shader(postprocessVertextShaderPath, postprocessFragmentShaderPath);
}

void PostProcessor::renderPostprocessed()
{
	// second pass.
	// draws the prerendered and post processed screen color texture onto a quad that fills the screen.

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer

	postprocessShader->useShader();

	glBindVertexArray(screenQuadVAO);
	glDisable(GL_DEPTH_TEST); // no need for depth testing since we just draw a single quad
	glBindTexture(GL_TEXTURE_2D, screenColorTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST); // reenable depth testing
	glBindVertexArray(0);

}

