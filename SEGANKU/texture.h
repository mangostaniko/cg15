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
 * note: use 8 bit RGB, no alpha channel !
 */
class Texture
{
	GLuint handle;
	const std::string filePath;

public:
	Texture(const std::string &filePath, bool alpha);
	~Texture();

	/**
	 * @brief binds this texture to the given opengl texture unit
	 * @param unit the opengl texture unit to bind to
	 */
	void bind(int unit);

	std::string getFilePath() const;
};

#endif // TEXTURE_H
