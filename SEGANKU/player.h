#ifndef PLAYER_H
#define PLAYER_H

#define GLM_SWIZZLE

#include <btBulletDynamicsCommon.h>
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
	// CAMERA SPECS 
	Camera *camera;
	GLFWwindow *window;
	glm::vec3 camDirection;
	glm::vec3 camRight;
	glm::vec3 camUp;
	glm::mat4 viewMat;
	glm::mat4 projMat;

	enum CameraNavigationMode
	{
		FOLLOW_PLAYER,
		FREE_FLY
	};

	static CameraNavigationMode cameraNavMode;
	CameraNavigationMode lastNavMode;
	glm::mat4 lastCamTransform; // safe transformation matrix when changing modes


	// PHYSICS SPECS
	btCollisionShape *playerShape;
	btRigidBody *playerBody;
	btDefaultMotionState *motionState;
	float timePassed = 0; // in seconds


	// GAMEPLAY SPECS
	// durations are given in seconds
	Geometry *currentFood = nullptr;	// pointer to the currently eaten carrot
	const float MAX_ANIM = 2.5;	// max length of animation
	const int NEEDED_FOOD = 10;	// how much food is needed to get through winter
	const int DEFENSE_FOOD_COST = 3;	// how many carrots are used in a skunk defense
	const float MAX_RUN_TIME = 4.0;	// max speed time
	const float BREAK_TIME = 5.0;	// speed cool off time
	const float DIGEST_TIME = 20.0;	// digestion time for 1 carrot
	const float DEFENSE_TIME = 3.0;	// how long the player is protected after defense activation

	int foodCount = 0;		    // how much food was already eaten
	bool fullStomach = false;   // true if we ate enough
	bool overWeight  = false;	// ate too much?
	bool canRun      = true;	// can use Speed?

	float animDur    = 0;		// timer for animation duration
	float runDur     = 0;		// timer for speed duration
	float breakDur   = 0;       // timer for speed cooloff duration
	float digestDur  = 0;		// timer for digestion duration
	float defenseDur = 0;       // timer for active defense duration

	bool inBush = false;		// true if currently in bush
	bool defenseActive = false; // true if skunk defense cloud active



	static double scrollY; // amount scrolled since last frame

	/**
	 * @brief check if the camera navigation mode has changed and set camera accordingly
	 * NOTE: currently only works if there are only 2 nav modes
	 */
	void handleNavModeChange();

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
	virtual void draw(Shader *shader, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat);

	/**
	 * @brief toggle the camera navigation mode
	 */
	void toggleNavMode();

	/**
	 * @brief get the current view matrix of the player camera
	 * @return the current view matrix
	 */
	glm::mat4 getViewMat();

	/**
	 * @brief get the current projection matrix of the player camera
	 * @return the current projection matrix
	 */
	glm::mat4 getProjMat();

	/**
	* @brief get the RigidBody of the Player for Bullet Collision
	* @return the btRigidBody 
	*/
	btRigidBody *getRigidBody();

	/**
	* @brief handle the event in which the Player collides with a food Geometry Object
	* @param the Geometry Object that is currently being eaten
	*/
	void eat(Geometry *carrot);

	/**
	* @brief Get the information if the Player has already eaten enough carrots
	* @return true if the player is already full
	*/
	bool isFull();

	/**
	* @brief get the information if the Player is currently hidden in a Bush Geometry Object
	* @return true if player is currently in a Bush, else false
	*/
	bool isInBush();

	/**
	* @brief check if the skunk defense cloud is currently active
	* @return true if skunk defense cloud is currently active
	*/
	bool isDefenseActive();

	/**
	 * @brief activate the skunk defense cloud for a certain time.
	 * @return whether the defense was successfully activated
	 */
	bool attemptDefenseActivation();

	/**
	* @brief get the number of currently eaten food objects
	* @return the number of currently eaten carrots
	*/
	int getFoodCount();

	/**
	* @brief set the information whether or not Player is currently in a Bush (called and set by Physics Class)
	* @param inB the information whether or not in bush
	*/
	void setInBush(bool inBush_);

	/**
	* @brief resets the Player for the start of a new game (reset number of carrots eaten)
	*/
	void resetPlayer();

	/**
	* @brief resets the Player for the start of a new game (reset number of carrots eaten)
	* @return whether or not the Player is currently eating
	*/
	bool isEating();

	/**
	* @brief Returns a String depending on the current food level (eg. "Still hungry", "Need more", "I'm so full")
	* @return whether or not the Player is currently eating
	*/
	std::string getFoodReaction();

};

#endif // PLAYER_H
