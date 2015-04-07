#include "cube.h"

#include <iostream>

const GLfloat Cube::vertices[] = {

    // positions are ordered counterclockwise

	//	 Position             Color
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f

};

const GLuint Cube::indices[] = {

    // counterclockwise order defines front-facing triangle
    // so seen from outside we should have counterclockwise order

    0, 1, 2, // test facing inside
    0, 2, 3, // test facing inside
    4, 5, 6,
    4, 6, 7,

    0, 1, 4,
    1, 5, 4,
	2, 3, 6,
    3, 7, 6,

    1, 2, 6,
    1, 6, 5,
    0, 4, 7,
    0, 7, 3

};

Cube::Cube(const glm::mat4 &modelMatrix_, Shader *shader_)
	: SceneObject(modelMatrix_)
    , shader(shader_)
{
	// copy positions to GL_ARRAY_BUFFER in vram. these are just the raw positions without defined topology.
	glGenBuffers(1, &vertexBuffer); // generate handle
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // bind to active context
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy data
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind buffer

	// copy indices to GL_ELEMENT_ARRAY_BUFFER in vram. these define the mesh structure.
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// generate vertex array object (vao) bindings. the vao encapsulates all references to relevant data
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// associate vertex array buffer data with shader program attributes
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	GLint positionAttribIndex = 0; // index of position attribute (layout defined in shader source file)
	GLint colorAttribIndex = 1; // index of color attribute

	glEnableVertexAttribArray(positionAttribIndex); // the vertex attrib array specifies shader input vertices
	glVertexAttribPointer(positionAttribIndex,   // index of vertex attribute to be modified
	                      3,                     // number of elements of the attrib (three for x, y, z)
	                      GL_FLOAT,              // type
	                      GL_FALSE,              // normalized?
	                      6*sizeof(float),       // size of whole vertex attrib array
	                      0                      // offset of this vertex attrib within the array
	                      ); // specify format of vertex attrib array to be associated with given shader variable

	glEnableVertexAttribArray(colorAttribIndex);
	glVertexAttribPointer(colorAttribIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));


	// unbind buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

Cube::~Cube()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vao);

}

void Cube::update()
{

}

void Cube::draw()
{
	glBindVertexArray(vao); // bind the vertex array used to supply vertices
	glDrawElements(GL_TRIANGLES, CUBE_INDEX_COUNT, GL_UNSIGNED_INT, 0); // use given indices
	glBindVertexArray(0);
}

