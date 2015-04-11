#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImagePlus.h>

#include <iostream>
#include <string>

/**
 * @brief Texture class.
 * This loads an opengl texture from an image file and stores a handle to it.
 */
class Texture
{
	GLuint handle;

public:
	Texture(const std::string &filePath);
	~Texture();

	void bind(int unit);
};

#endif // TEXTURE_H
