#include "textrenderer.h"

TextRenderer::TextRenderer(const std::string &fontPath, const GLuint &windowWidth, const GLuint &windowHeight)
{
	// set OpenGL options.
	// we need blending for the text alpha (disable to see the quads onto which the glyph textures are drawn)
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// compile and configure text shader
	// note that we use an orthographic projection matrix defining the origin at the bottom left of the screen
	textShader = new Shader("../SEGANKU/shaders/text.vert", "../SEGANKU/shaders/text.frag");
	glm::mat4 projMat = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
	projMat = glm::translate(projMat, glm::vec3(0, 0, 1)); // closer to camera to make sure it doesnt get occluded
	textShader->useShader();
	glUniformMatrix4fv(glGetUniformLocation(textShader->programHandle, "projMat"), 1, GL_FALSE, glm::value_ptr(projMat));

	// load FreeType glyphs for each character of 7-bit ASCII and create opengl textures from glyph bitmaps
	// the resulting Glyph structs (texture and glyph metrics) are stored in the glyphs map
	loadGlyphs(fontPath);

	// generate vao and vbo handles
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// setup bindings associated with this vao
	// note that we use quads to draw glyph textures, drawn as 2 triangles, so we have 6 vertices.
	// for each vertex we store the x and y position and the uv coordinates, thus 4 floats.
	// we do not yet copy the data, so the memory space is reserved but uninitialized!
	// DYNAMIC usage hint is given to GL implementation, since data will be 'modified repeatedly and used many times'.
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	GLint vertexAttribIndex = 0;
	glVertexAttribPointer(vertexAttribIndex, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

TextRenderer::~TextRenderer()
{
	delete textShader; textShader = nullptr;
}

void TextRenderer::renderText(const std::string &text, GLfloat x, GLfloat y, GLfloat scaleFactor, const glm::vec3 &color)
{
	// set bindings
	textShader->useShader();
	glUniform3f(glGetUniformLocation(textShader->programHandle, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	// for each character in the text, get the corresponding glyph and render its texture to a quad
	std::string::const_iterator character;
	for (character = text.begin(); character != text.end(); ++character) {

		Glyph glyph = glyphs[*character]; // get preloaded glyph struct from the map

		GLfloat xmin = x + glyph.bearing.x * scaleFactor; // horizontal bearing is distance from origin to left of glyph
		GLfloat ymin = y - (glyph.size.y - glyph.bearing.y) * scaleFactor; // vertical bearing is baseline to top (size can be bigger, so ymin can be below baseline)

		GLfloat width = glyph.size.x * scaleFactor;
		GLfloat height = glyph.size.y * scaleFactor;

		// define quad vertices (2 triangles)
		GLfloat quadVertices[6][4] = {
		    { xmin,          ymin + height,  0.0, 0.0 },
		    { xmin,          ymin,           0.0, 1.0 },
		    { xmin + width,  ymin,           1.0, 1.0 },

		    { xmin,          ymin + height,  0.0, 0.0 },
		    { xmin + width,  ymin,           1.0, 1.0 },
		    { xmin + width,  ymin + height,  1.0, 0.0 }
		};

		// render the glyph opengl texture onto the quad
		glBindTexture(GL_TEXTURE_2D, glyph.textureId);

		// update vbo memory with new quad vertex data
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices), quadVertices); // use glBufferSubData instead of glBufferData
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// draw the quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// advance xmin to next position as defined in Glyph struct
		x += (glyph.advance >> 6) * scaleFactor; // division by 64 since Glyph.advance is defined in 1/64 pixels

	}


	// unbind
	glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void TextRenderer::loadGlyphs(const std::string &fontPath)
{
	// init FreeType library object
	// NOTE: FreeType functions return a value different from 0 if an error occurred
	// i.e. the 'root' of a set of fonts, faces, sizes, etc.
	// also includes a memory manager and a font rasterizer
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
	    std::cerr << "ERROR FREETYPE: Could not init FreeType Library." << std::endl;
	}

	// create a new FreeType typeface
	FT_Face typeface;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &typeface)) {
	    std::cerr << "ERROR FREETYPE: Failed to load font '" << fontPath << "'." << std::endl;
	}

	FT_Set_Pixel_Sizes(typeface, 0, 48); // width set to 0 will let it be autogenerated based on height

	// disable restriction on byte-alignment for the start of each pixel row in memory
	// note that GL_UNPACK_ALIGNMENT is about how data is read, not written.
	// default is 4 bytes since textures often use 4 channels of 8 bit per pixel, so the row size will be a multiple of 4 bytes
	// but for the glyphs they are just grayscale 8 bit per pixel, so the row size / offset can be of any size in bytes,
	// thus to avoid segmentation faults we need greater access precision.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load glyphs for each character of 7-bit ASCII and store in a map
	for (GLubyte character = 0; character < 128; ++character)
	{
		// load glyph related to given character and render to bitmap
	    if (FT_Load_Char(typeface, character, FT_LOAD_RENDER)) {
			std::cerr << "ERROR FREETYPE: Failed to load glyph for character '" << character << "'." << std::endl;
	        continue;
	    }

	    // generate opengl texture from glyph bitmap
		// note that FreeType glyph bitmaps are 8-bit grayscale
		// so we just store them in one of the 8-bit channels, here GL_RED is used
	    GLuint texture;
	    glGenTextures(1, &texture);
	    glBindTexture(GL_TEXTURE_2D, texture);
	    glTexImage2D(
	        GL_TEXTURE_2D,
	        0,
	        GL_RED,
	        typeface->glyph->bitmap.width,
	        typeface->glyph->bitmap.rows,
	        0,
	        GL_RED,
	        GL_UNSIGNED_BYTE,
	        typeface->glyph->bitmap.buffer
	    );

	    // set texture parameters
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    // store glyph texture and defining parameters in map
	    Glyph glyph = {
	        texture,
	        glm::ivec2(typeface->glyph->bitmap.width, typeface->glyph->bitmap.rows),
	        glm::ivec2(typeface->glyph->bitmap_left, typeface->glyph->bitmap_top),
	        GLuint(typeface->glyph->advance.x)
	    };

	    glyphs.insert(std::pair<GLchar, Glyph>(character, glyph));
	}

	// FreeType cleanup functions
	FT_Done_Face(typeface);
	FT_Done_FreeType(ft);

}
