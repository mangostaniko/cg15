#include "texture.h"

Texture::Texture(const std::string &filePath_)
	: filePath(filePath_)
{
	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0); // select the active texture unit of the context
	glBindTexture(GL_TEXTURE_2D, handle);

	// load image from file using FreeImagePlus (the FreeImage C++ wrapper)
	fipImage img;
	if (!img.load(filePath.c_str(), 0)) {
		std::cerr << "ERROR: FreeImage could not load image file '" << filePath << "'." << std::endl;
	}

	// specify a texture of the active texture unit at given target
	// a unit can contain multiple texture targets, but recommended to use only one per unit
	// parameters: target, mipmap level, internal format, width, heigth, border width, internal format, data format, image data
	// note: for some reason it seems that 8 bit RGB images are really stored in BGR format.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, img.accessPixels());

	// automatically generate mipmaps (mip = multum in parvo, i.e. 'much in little')
	// mipmaps are filtered and downsampled copies of the texture stored compactly in a single file,
	// used to avoid aliasing effects when the sampling rate is too low for the texture frequency
	// e.g. for far away surfaces. by taking a filtered average it doesnt matter where the sample hits.
	glGenerateMipmap(GL_TEXTURE_2D);

	// set texture minification and magnification filters
	// minification:  how to filter downsampled texture when there's not enough space
	// magnification: how to interpolate texture to fill remaining space
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

Texture::~Texture()
{
	glDeleteTextures(1, &handle);
}

void Texture::bind(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, handle);
}

std::string Texture::getFilePath() const
{
	return filePath;
}

