#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>

#include "sceneobject.h"
#include "surface.h"
#include "shader.h"
#include "texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/**
 * @brief A Geometry is a SceneObject that holds Surfaces which contain mesh data and textures.
 */
class Geometry : public SceneObject
{

	// surfaces store mesh data and textures
	std::vector<std::shared_ptr<Surface>> surfaces;

	// the path of the directory containing the model file to load
	std::string directoryPath;

	/**
	 * @brief load surfaces from file
	 * note: this loads only the first diffuse, specular and normal texture for each surface
	 * and stores them in this order in the surface
	 * @param filePath the path of the file to load surfaces from
	 */
	void loadSurfaces(const std::string &filePath);

	/**
	 * @brief process all meshes contained in given node
	 * and recursively process all child nodes
	 * @param node the current node to process
	 * @param scene the aiScene containing the node
	 */
	void processNode(aiNode *node, const aiScene *scene);

	/**
	 * @brief load data from assimp aiMesh to new Surface object
	 * note: this loads only the first diffuse, specular and normal texture for each surface
	 * and stores them in this order in the surface
	 * @param mesh the aiMesh to process
	 * @param scene the aiScene containing the mesh
	 * @return
	 */
	void processMesh(aiMesh *mesh, const aiScene *scene);

public:

	Geometry(const glm::mat4 &matrix_, const std::string &filePath);
	virtual ~Geometry();

	virtual void update(float timeDelta);

	/**
	 * @brief draw the SceneObject using given shader
	 */
	virtual void draw(Shader *shader);

};

#endif // GEOMETRY_H
