#ifndef PLAYER_H
#define PLAYER_H

#define GLM_SWIZZLE

#include "geometry.h"
#include "camera.h"

#define ZOOM_MIN 30.0f
#define ZOOM_MAX 80.0f

/**
 * @brief The Player class. This stores the player Geometry and a Camera,
 * as well as a GLFWwindow to handle input.
 */
class Player : public Geometry
{
	Camera *camera;
	GLFWwindow *window;
	glm::vec3 camDirection;
	glm::vec3 camRight;
	glm::vec3 camUp;
	glm::mat4 viewProjMat;

	enum CameraNavigationMode
	{
		FOLLOW_PLAYER,
		FREE_FLY
	};

	static CameraNavigationMode cameraNavMode;
	CameraNavigationMode lastNavMode;
	glm::mat4 lastCamTransform; // safe transformation matrix when changing modes

	static double scrollY; // amount scrolled since last frame

	/**
	 * @brief check if the camera navigation mode has changed and set camera accordingly
	 * NOTE: currently only works if there are only 2 nav modes
	 */
	void handleNavModeChange();

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
	Player(const glm::mat4 &matrix_, Camera *camera_, GLFWwindow *window_, const std::string &filePath);
	virtual ~Player();

	virtual void update(float timeDelta);
	virtual void draw(Shader *shader);

};

#endif // PLAYER_H
