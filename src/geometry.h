#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "sceneobject.h"
#include "shader.h"
#include "texture.h"

class Geometry : public SceneObject
{
	GLuint vao;
	GLuint positionBuffer, normalBuffer, uvBuffer, indexBuffer;
	Shader *shader;
	Texture *texture;

	std::vector<GLfloat> positions; // vertex positions
	std::vector<GLfloat> normals; // vertex normals
	std::vector<GLfloat> uvs; // vertex uvs
	std::vector<GLuint> indices; // indices associate vertices to define mesh topology

	// TEMPORARY solution
	void init();

protected:
	// TEMPORARY used to create subclasses that contain the data.
	// this is just a temporary solution!!
	Geometry(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_,
	         const std::vector<GLfloat> &positions_,
	         const std::vector<GLfloat> &normals_,
	         const std::vector<GLfloat> &uvs_,
	         const std::vector<GLuint> &indices_);

public:
	Geometry(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_);
	virtual ~Geometry();

	virtual void update(float timeDelta) = 0;
	virtual void draw();

	// TEMPORARY the following are placeholders to fill in data in a given format.
	// TODO: replace with proper object loading from open standars like COLLADA
	// TODO: comments
	void setPositions(const std::vector<GLfloat> &positions_);
	void setNormals(const std::vector<GLfloat> &normals_);
	void setUVs(const std::vector<GLfloat> &uvs_);
	void setIndices(const std::vector<GLuint> &indices_);

};

#endif // GEOMETRY_H
