#ifndef CUBE_H
#define CUBE_H

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include "../sceneobject.h"
#include "../shader.h"
#include "../texture.h"

#define CUBE_VERTEX_COUNT 24
#define CUBE_INDEX_COUNT 36

class Cube : public SceneObject
{
	GLuint vao;
	GLuint positionBuffer, normalBuffer, uvBuffer, indexBuffer;
	Shader *shader;
	Texture *texture;

	static const GLfloat positions[CUBE_VERTEX_COUNT * 3]; // raw vertex data
	static const GLfloat normals[CUBE_VERTEX_COUNT * 3];
	static const GLfloat uvs[CUBE_VERTEX_COUNT * 2];
	static const GLuint indices[CUBE_INDEX_COUNT]; // indices define mesh topology


public:
	Cube(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_);
	virtual ~Cube();

	virtual void update(float timeDelta);
	virtual void draw();
};

#endif // CUBE_H
