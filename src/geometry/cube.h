#ifndef CUBE_H
#define CUBE_H

#include "iterator"

#include "../geometry.h"

#define CUBE_VERTEX_COUNT 24
#define CUBE_INDEX_COUNT 36

/**
 * @brief This really just a container providing data for the Geometry base class.
 * This will be replace with loading from COLLADA files later.
 */
class Cube : public Geometry
{

	static const GLfloat positionsData[CUBE_VERTEX_COUNT * 3];
	static const GLfloat normalsData[CUBE_VERTEX_COUNT * 3];
	static const GLfloat uvsData[CUBE_VERTEX_COUNT * 2];
	static const GLuint indicesData[CUBE_INDEX_COUNT];


public:
	Cube(const glm::mat4 &matrix_);
	virtual ~Cube();

	virtual void update(float timeDelta);
};

#endif // CUBE_H
