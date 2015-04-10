#include "cube.h"

#include <iostream>

Cube::Cube(const glm::mat4 &modelMatrix_, Shader *shader_, Texture *texture_)
	: SceneObject(modelMatrix_)
    , shader(shader_)
    , texture(texture_)
{
	// copy positions to GL_ARRAY_BUFFER in vram. these are just the raw positions without defined topology.
	glGenBuffers(1, &positionBuffer); // generate handle
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer); // bind to active context
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW); // copy data
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind buffer

	// copy normals to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy uvs to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy indices to GL_ELEMENT_ARRAY_BUFFER in vram. these define the mesh structure.
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// generate vertex array object (vao) bindings. the vao simply stores the state of the bindings that follow
	// so that they can be reactived quickly later, instead of doing it all over again
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// enable shader attributes at given indices to supply vertex data to them
	// the indices/layout of the shader attribute are defined in the shader source file
	GLint positionAttribIndex   = 0;
	GLint normalAttribIndex     = 1;
	GLint uvAttribIndex         = 2;
	glEnableVertexAttribArray(positionAttribIndex);
	glEnableVertexAttribArray(normalAttribIndex);
	glEnableVertexAttribArray(uvAttribIndex);

	// associate data from current bound array buffer with shader program attributes
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer); // the buffer contains the data
	glVertexAttribPointer(positionAttribIndex,   // index of vertex attribute to be modified
	                      3,                     // number of elements of the attrib (three for x, y, z)
	                      GL_FLOAT,              // type
	                      GL_FALSE,              // normalized?
	                      0,                     // size of whole vertex attrib array (if offset used)
	                      0                      // offset of this vertex attrib within the array
	                      ); // specify format of vertex attrib array to be associated with given shader attribute

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(uvAttribIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// bind index buffer to the element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// unbind vao. the state of bindings until here are stored in vao.
	glBindVertexArray(0);

	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

Cube::~Cube()
{
	// delete buffers (free vram)
	glDeleteBuffers(1, &positionBuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vao);

}

void Cube::update(float timeDelta)
{
	// rotate cube a given angle per second around given axis
	modelMatrix = glm::rotate(modelMatrix, timeDelta, glm::vec3(0, 1, 0));
}

void Cube::draw()
{
	// pass texture to shader
	texture->bind(0); // bind texture to texture unit 0
	GLint texLocation = glGetUniformLocation(shader->programHandle, "colorTexture"); // get uniform location in shader
	glUniform1i(texLocation, 0); // associate texture unit 0 with the shader uniform

	// pass model matrix to shader
	GLint modelMatLocation = glGetUniformLocation(shader->programHandle, "modelMat"); // get uniform location in shader
	glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix)); // shader location, count, transpose?, value pointer

	// draw triangles from given indices
	glBindVertexArray(vao); // bind the vertex array used to supply vertices
	glDrawElements(GL_TRIANGLES, CUBE_INDEX_COUNT, GL_UNSIGNED_INT, 0); // use given indices
	glBindVertexArray(0);
}


///////////////////////
/// DATA DEFINITION
///////////////////////

const GLfloat Cube::positions[] = {

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

const GLfloat Cube::normals[] = {

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

const GLfloat Cube::uvs[] = {

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

const GLuint Cube::indices[] = {

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

