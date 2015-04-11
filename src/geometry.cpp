#include "geometry.h"

Geometry::Geometry(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_, const std::vector<GLfloat> &positions_, const std::vector<GLfloat> &normals_, const std::vector<GLfloat> &uvs_, const std::vector<GLuint> &indices_)
    : SceneObject(matrix_)
    , shader(shader_)
    , texture(texture_)
    , positions(positions_)
    , normals(normals_)
    , uvs(uvs_)
    , indices(indices_)
{
	init();
}

Geometry::Geometry(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_)
    : SceneObject(matrix_)
    , shader(shader_)
    , texture(texture_)
{
	init();
}

void Geometry::init()
{
	// copy positions to GL_ARRAY_BUFFER in vram. these are just the raw positions without defined topology.
	glGenBuffers(1, &positionBuffer); // generate handle
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer); // bind to active context
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), &positions[0], GL_STATIC_DRAW); // copy data
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind buffer

	// copy normals to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy uvs to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(GLfloat), &uvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy indices to GL_ELEMENT_ARRAY_BUFFER in vram. these define the mesh structure.
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
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

Geometry::~Geometry()
{
	// delete buffers (free vram)
	glDeleteBuffers(1, &positionBuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vao);
}

void Geometry::draw()
{
	// pass texture to shader
	texture->bind(0); // bind texture to texture unit 0
	GLint texLocation = glGetUniformLocation(shader->programHandle, "colorTexture"); // get uniform location in shader
	glUniform1i(texLocation, 0); // associate texture unit 0 with the shader uniform

	// pass model matrix to shader
	GLint modelMatLocation = glGetUniformLocation(shader->programHandle, "modelMat"); // get uniform location in shader
	glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(getMatrix())); // shader location, count, transpose?, value pointer

	// draw triangles from given indices
	glBindVertexArray(vao); // bind the vertex array used to supply vertices
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // use given indices
	glBindVertexArray(0);
}

void Geometry::setPositions(const std::vector<GLfloat> &positions_)
{
	positions = positions_;
}

void Geometry::setNormals(const std::vector<GLfloat> &normals_)
{
	normals = normals_;
}

void Geometry::setUVs(const std::vector<GLfloat> &uvs_)
{
	uvs = uvs_;
}

void Geometry::setIndices(const std::vector<GLuint> &indices_)
{
	indices = indices_;
}
