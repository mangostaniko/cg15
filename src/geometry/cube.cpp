#include "cube.h"

Cube::Cube(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_)
	: Geometry(matrix_, shader_, texture_
    , std::vector<GLfloat>(std::begin(positionsData), std::end(positionsData))
    , std::vector<GLfloat>(std::begin(normalsData), std::end(normalsData))
    , std::vector<GLfloat>(std::begin(uvsData), std::end(uvsData))
    , std::vector<GLuint>(std::begin(indicesData), std::end(indicesData)))
{
}

Cube::~Cube()
{

}

void Cube::update(float timeDelta)
{
	// rotate cube a given angle per second around given axis
	rotate(timeDelta, LEFT, glm::vec3(1, 1, 2));
}


///////////////////////
/// DATA DEFINITION
///////////////////////

const GLfloat Cube::positionsData[] = {

    // positions are ordered counterclockwise

    // note that we need to spefify positions twice since we want
    // different normal attributes for different sides of the cubes to have sharp edges

    // back
	-0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,

    // front
    -0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

    // top
     0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f,  0.5f,

    // bottom
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,

    // left
    -0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,

	// right
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f

};

const GLfloat Cube::normalsData[] = {

    // different normals are used for eaech side to have sharp edges

	// back
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,

	// front
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,

	// top
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,

	// bottom
	 0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
	 0.0f, -1.0f,  0.0f,
	 0.0f, -1.0f,  0.0f,

	// left
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

	// right
	 1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
	 1.0f,  0.0f,  0.0f,
	 1.0f,  0.0f,  0.0f

};

const GLfloat Cube::uvsData[] = {

	// back
	 1.0f,  0.0f,
	 1.0f,  1.0f,
	 0.0f,  1.0f,
	 0.0f,  0.0f,

	// front
     0.0f,  0.0f,
     1.0f,  0.0f,
     1.0f,  1.0f,
     0.0f,  1.0f,

	// top
     1.0f,  0.0f,
     1.0f,  1.0f,
     0.0f,  1.0f,
     0.0f,  0.0f,

	// bottom
	 0.0f,  0.0f,
     1.0f,  0.0f,
	 1.0f,  1.0f,
	 0.0f,  1.0f,

	// left
     1.0f,  0.0f,
     1.0f,  1.0f,
     0.0f,  1.0f,
     0.0f,  0.0f,

	// right
	 0.0f,  0.0f,
     1.0f,  0.0f,
	 1.0f,  1.0f,
	 0.0f,  1.0f

};

const GLuint Cube::indicesData[] = {

    // counterclockwise order defines front-facing triangle
    // so seen from outside we should have counterclockwise order

    // back
    0, 1, 2,
    0, 2, 3,

    // front
    4, 5, 6,
    4, 6, 7,

	// top
    8, 9, 10,
    8, 10, 11,

    // bottom
	12, 13, 14,
    12, 14, 15,

    // left
    20, 21, 22,
    20, 22, 23,

    // right
    16, 17, 18,
    16, 18, 19

};

