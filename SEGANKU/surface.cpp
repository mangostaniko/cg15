#include "surface.h"

Surface::Surface(const std::vector<Vertex> &vertices_, const std::vector<GLuint> &indices_, const std::shared_ptr<Texture> &texDiffuse_, const std::shared_ptr<Texture> &texSpecular_, const std::shared_ptr<Texture> &texNormal_)
    : vertices(vertices_)
	, indices(indices_)
	, texDiffuse(texDiffuse_)
	, texSpecular(texSpecular_)
	, texNormal(texNormal_)
{
	init();
}

void Surface::init()
{
	// generate vertex array object (vao) bindings. the vao simply stores the state of the bindings that follow
	// so that they can be reactived quickly later, instead of doing it all over again
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// copy vertex data to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // bind to active context
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // copy data

	// copy indices to GL_ELEMENT_ARRAY_BUFFER in vram. these define the mesh structure.
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// enable shader attributes at given indices to supply vertex data to them
	// the indices/layout of the shader attribute are defined in the shader source file
	GLint positionAttribIndex   = 0;
	GLint normalAttribIndex     = 1;
	GLint uvAttribIndex         = 2;
	glEnableVertexAttribArray(positionAttribIndex);
	glEnableVertexAttribArray(normalAttribIndex);
	glEnableVertexAttribArray(uvAttribIndex);

	// associate data from current bound array buffer with shader program attributes
	// and specify the format of the vertex attrib array data
	// PARAMETERS:
	// attrib index in shader, num elements, type, normalized?, vertex attrib array size, offset within the array
	glVertexAttribPointer(positionAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glVertexAttribPointer(uvAttribIndex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));

	// unbind vao. the state of bindings until here are stored in vao.
	glBindVertexArray(0);

	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

Surface::~Surface()
{
	// delete buffers (free vram)
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);

	glDeleteVertexArrays(1, &vao);
}

void Surface::draw(Shader *shader)
{
	// pass textures to shader
	// for now just uses the diffuse, specular and normal textures.

	if (texDiffuse) {
		texDiffuse->bind(0); // bind texture to texture unit 0
		GLint diffuseTexLocation = glGetUniformLocation(shader->programHandle, "diffuseTexture"); // get uniform location in shader
		glUniform1i(diffuseTexLocation, 0); // associate texture unit 0 with the shader uniform
	}
	if (texSpecular) {
		texSpecular->bind(1);
		GLint specularTexLocation = glGetUniformLocation(shader->programHandle, "specularTexture");
		glUniform1i(specularTexLocation, 1);
	}
	if (texNormal) {
		texNormal->bind(2);
		GLint normalsTexLocation = glGetUniformLocation(shader->programHandle, "normalTexture");
		glUniform1i(normalsTexLocation, 2);
	}

	// draw triangles from given indices
	glBindVertexArray(vao); // bind the vertex array used to supply vertices
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // use given indices
	glBindVertexArray(0);

	// DEBUG PRINT VERTICES
	//std::cout << vertices.size() << std::endl;
	/*//
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		std::cout << "pos " << vertices[i].position[0] << vertices[i].position[1] << vertices[i].position[2] << std::endl;
		std::cout << "normal " << vertices[i].normal[0] << vertices[i].normal[1] << vertices[i].normal[2] << std::endl;
		std::cout << "uv " << vertices[i].uv[0] << vertices[i].uv[1] << std::endl;

	}
	//*/

}
