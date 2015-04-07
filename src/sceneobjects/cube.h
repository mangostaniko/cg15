#ifndef CUBE_H
#define CUBE_H

#include "../sceneobject.h"
#include "../shader.h"

#define CUBE_VERTEX_COUNT 8
#define CUBE_INDEX_COUNT 36

class Cube : public SceneObject
{
	GLuint vao;
	GLuint vertexBuffer, indexBuffer;
	Shader *shader;

	static const GLfloat vertices[CUBE_VERTEX_COUNT * 6]; // raw vertex data
	static const GLuint indices[CUBE_INDEX_COUNT]; // indices define mesh topology

public:
	Cube(const glm::mat4 &modelMatrix_, Shader *shader_);
	virtual ~Cube();

	virtual void update();
	virtual void draw();
};

#endif // CUBE_H
