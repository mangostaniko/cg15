#include "geometry.h"

Geometry::Geometry(const glm::mat4 &matrix_, const std::string &filePath)
    : SceneObject(matrix_)
{
	loadSurfaces(filePath);
}

Geometry::~Geometry()
{

}

void Geometry::update(float timeDelta)
{

}

void Geometry::draw(Shader *shader)
{
	// pass model matrix to shader
	GLint modelMatLocation = glGetUniformLocation(shader->programHandle, "modelMat"); // get uniform location in shader
	glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(getMatrix())); // shader location, count, transpose?, value pointer

	for (GLuint i = 0; i < surfaces.size(); ++i) {
		surfaces[i]->draw(shader);
	}

}

void Geometry::loadSurfaces(const std::string &filePath)
{

	// read surface data from file using Assimp.
	//
	// IMPORTANT ASSIMP POSTPROCESS FLAGS
	// - aiProcess_PreTransformVertices: to load vertices in world space i.e. apply transformation matrices, which we dont load
	// - aiProcess_Triangulate: needed for OpenGL
	// if there are problems with the uvs, try aiProcess_FlipUVs
	// note: experiment with flags like aiProcess_SplitLargeMeshes, aiProcess_OptimizeMeshes, when using bigger models.
    Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filePath, aiProcess_PreTransformVertices | aiProcess_Triangulate);

    // check for errors
	if (!scene || !scene->mRootNode || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR ASSIMP: " << importer.GetErrorString() << std::endl;
        return;
    }

    // save path to the directory containing the file
    directoryPath = filePath.substr(0, filePath.find_last_of('/'));

    // recursively process Assimp root node
	processNode(scene->mRootNode, scene);
}

void Geometry::processNode(aiNode *node, const aiScene *scene)
{
	// process all meshes contained in this node.
	// note that the node->mMeshes just define the hierarchy
	// and store indices to the actual data in scene->mMeshes
    for (GLuint i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    // then process all child nodes
    for (GLuint i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }

}

void Geometry::processMesh(aiMesh *mesh, const aiScene *scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<std::shared_ptr<Texture>> surfaceTextures;

	// process mesh vertices (positions, normals, uvs)
	for (GLuint i = 0; i < mesh->mNumVertices; ++i) {

		Vertex vertex;
		glm::vec3 vector;

		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		// uvs
		// note: while meshes can have 8 uv channels in assimp, we only take data from first uv channel.
		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.uv = vec;
		}
		else {
			vertex.uv = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	// process mesh faces and store indices
	for (GLuint i = 0; i < mesh->mNumFaces; ++i) {

		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// process material and store textures
	// note: we only load the first diffuse, specular and normal texture reffered to by the assimp material
	// and store them in this order
	if (mesh->mMaterialIndex >= 0) {

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::shared_ptr<Texture> texture;

		// load diffuse texture
		texture = loadMaterialTexture(material, aiTextureType_DIFFUSE);
		if (texture) { surfaceTextures.push_back(texture); }

		// load specular texture
		texture = loadMaterialTexture(material, aiTextureType_SPECULAR);
		if (texture) { surfaceTextures.push_back(texture); }

		// load normals texture
		texture = loadMaterialTexture(material, aiTextureType_NORMALS);
		if (texture) { surfaceTextures.push_back(texture); }

	}

	// return a Surface object created from the extracted aiMesh data
	surfaces.push_back(std::make_shared<Surface>(vertices, indices, surfaceTextures));
}

std::shared_ptr<Texture> Geometry::loadMaterialTexture(aiMaterial *mat, aiTextureType type)
{
	aiString texturePath;
	std::shared_ptr<Texture> texture = nullptr;

	if (mat->GetTexture(type, 0, &texturePath) == AI_SUCCESS) {

		// check if we already loaded the texture of the given path for another mesh
		bool skip = false;
		for (auto existingTexture : loadedTextures) {
			std::cout << existingTexture->getFilePath() << std::endl;
			std::cout << texturePath.C_Str() << std::endl;
	        if (existingTexture->getFilePath() == texturePath.C_Str()) {
				skip = true;
	            texture = existingTexture; // use pointer to existing texture
				break;
	        }
	    }

		// otherwise load the texture from the file
		if (!skip) {
			loadedTextures.push_back(std::make_shared<Texture>(directoryPath + '/' + texturePath.C_Str()));
			std::cout << "loaded texture: " << texturePath.C_Str() << std::endl;
			texture = loadedTextures.back();
		}

	}

	return texture;
}
