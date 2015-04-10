#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <glm/glm.hpp>

class SceneObject
{
private:

	glm::mat4 modelMatrix;

	glm::mat4 inverseMatrix;

public:
	SceneObject(const glm::mat4 &modelMatrix_);
	virtual ~SceneObject();

	
	/**
	 * @brief enum that specifies the order of matrix multiplication
	 */
	enum Order {LEFT, RIGHT};

	/**
	 * @brief update the state of the SceneObject
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	virtual void update(float timeDelta) = 0;

	/**
	 * @brief draw the SceneObject
	 */
	virtual void draw() = 0;

	/**
	 * @brief apply a transformation matrix to the current matrix
	 * @param transform_ matrix defining a transformation
	 * @param inverse_ the inverse matrix of transform
	 * @param multOrder order of multiplication
	 */
	void applyTransformation(const glm::mat4 &transform_, const glm::mat4 &inverse_, Order multOrder);

	/**
	 * @return the const reference to the current model matrix
	 */
	const glm::mat4& getMatrix() const;

	/**
     * @brief replaces current matrix and sets its inverse
	 * @param matrix_ the new transformation matrix
	 */
	void setTransform(const glm::mat4 &matrix_);

	/**
	* @return the constant reference to the inverse matrix of the current model matrix
	*/
	const glm::mat4& getInverseMatrix() const;

	/**
	* @brief applies an X axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateX(float radians, Order multOrder);

	/**
	* @brief applies a Y axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateY(float radians, Order multOrder);

	/**
	* @brief applies a Z axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateZ(float radians, Order multOrder);

	/**
	* @brief applies a translation operation to the current transformation
	* @param t translation vector
	* @param multOrder order of multiplication
	*/
	void translate(const glm::vec3 &t_, Order multOrder);

	/**
	 * @brief applies a scale operation to the current transformation 
	 * @param s scaling vector
	 * @param multOrder order of multiplication
	 */
	void scale(const glm::vec3 &s_, Order multOrder);
};

#endif // SCENEOBJECT_H
