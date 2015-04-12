#ifndef PLAYER_H
#define PLAYER_H

#define GLM_SWIZZLE

#include "cube.h"
#include "../camera.h"

/**
 * @brief The Player class. This stores the player Geometry and a Camera,
 * as well as a GLFWwindow to handle input.
 *
 * !!! TODO !!!: to have something to draw the player class currently inherits from Cube.
 * This should be changed to Geometry when collada import is implemented.
 */
class Player : public Cube
{
	Camera *camera;
	GLFWwindow *window;

	enum CameraNavigationMode
	{
		FOLLOW_PLAYER,
		FREE_FLY
	};

	static CameraNavigationMode cameraNavMode;

	static double scrollY; // amount scrolled since last frame

	/**
	 * @brief glfw keyCallback on key events
	 * @param window pointer to active window
	 * @param key the key code of the key that caused the event
	 * @param scancode a system and platform specific constant
	 * @param action type of key event: GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
	 * @param mods modifier keys held down on event
	 */
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

	/**
	 * @brief glfw callback on mouse scroll
	 * @param window pointer to active window
	 * @param deltaX the scroll delta on scroll axis X
	 * @param deltaY the scroll delta on scroll axis Y
	 */
	static void onScroll(GLFWwindow *window, double deltaX, double deltaY);

	/**
	 * @brief handle input to control player and camera.
	 * the camera follows the player geometry and looks at it, while allowing to be rotated around it.
	 * @param window pointer to active window
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	void handleInput(GLFWwindow *window, float timeDelta);

	/**
	 * @brief handle input to control camera in free fly mode.
	 * @param window pointer to active window
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	void handleInputFreeCamera(GLFWwindow *window, float timeDelta);


public:
	Player(const glm::mat4 &matrix_, Shader *shader_, Texture *texture_, Camera *camera_, GLFWwindow *window_);
	virtual ~Player();

	virtual void update(float timeDelta);

};

#endif // PLAYER_H
