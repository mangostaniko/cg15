#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImagePlus.h>

#include <iostream>
#include <string>

class Texture
{
	GLuint handle;

public:
	Texture(const std::string &filePath);
	~Texture();

	void bind(int unit);
};

#endif // TEXTURE_H
