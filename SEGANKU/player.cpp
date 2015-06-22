#include "player.h"

Player::CameraNavigationMode Player::cameraNavMode = FOLLOW_PLAYER;
double Player::scrollY = 0.0;

Player::Player(const glm::mat4 &matrix_, Camera *camera_, GLFWwindow *window_, const std::string &filePath)
    : Geometry(matrix_, filePath)
    , camera(camera_)    
    , window(window_)
{
	// set glfw callbacks
	glfwSetScrollCallback(window, onScroll);

	camera->setTransform(glm::translate(glm::mat4(1.0f), getLocation()+glm::vec3(0,2,6)));  //move camera back a bit
	lastCamTransform = camera->getMatrix();
	camDirection = glm::normalize(camera->getLocation() - getLocation());
	camUp = glm::vec3(0, 1, 0);
	camRight = glm::normalize(glm::cross(camUp, camDirection));

	playerShape = new btSphereShape(1);
	btTransform playerTransform;
	playerTransform.setIdentity();
	playerTransform.setOrigin(btVector3(getLocation().x, getLocation().y+0.5, getLocation().z));

	btScalar mass(3.0); btVector3 inertia(0, 0, 0);
	playerShape->calculateLocalInertia(mass, inertia);

	motionState = new btDefaultMotionState(playerTransform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, playerShape, inertia);
	playerBody = new btRigidBody(info);
	playerBody->setActivationState(DISABLE_DEACTIVATION);
	playerBody->setCollisionFlags(playerBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	playerBody->setUserPointer(this);
	playerBody->setFriction(10);
	//playerBody->setDamping(3, 3);
}

Player::~Player()
{
	delete camera; camera = nullptr;
	window = nullptr;
}

void Player::update(float timeDelta)
{
	if (foodCount >= NEEDED_FOOD) {
		fullStomach = true;
	}

	if (currentFood != nullptr) {
		if (animDur < MAX_ANIM) {
			animDur += timeDelta;
			currentFood->setLocation(getLocation() + glm::vec3(0, 2, 0));
			currentFood->rotateY(glm::radians(180.0) * timeDelta, SceneObject::RIGHT);
		}
		else {
			animDur = 0;
			currentFood->setLocation(glm::vec3(300, -300, 300));
			currentFood = nullptr;
		}
	}

	// note: camera navigation mode is toggled on tab key press, look for keyCallback
	handleNavModeChange();

	if (cameraNavMode == FOLLOW_PLAYER) {

		handleInput(window, timeDelta);
		glm::vec3 v = glm::normalize(getLocation() - camera->getLocation()) * 5.0f;
		viewMat = glm::lookAt(getLocation()-v, getLocation() + glm::vec3(0, 1, 0), camUp);
	}
	else {

		handleInputFreeCamera(window, timeDelta);
		viewMat = camera->getViewMatrix();
	}

	projMat = camera->getProjectionMatrix();

	// handle skunk defense duration
	if (defenseActive) {

		defenseDur += timeDelta;

		if (defenseDur > DEFENSE_TIME) {
			defenseActive = false;
			defenseDur = 0;
		}
	}

}

void Player::draw(Shader *shader, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat)
{
	Geometry::draw(shader, camera, useFrustumCulling, filterType, viewMat);
}

void Player::handleInput(GLFWwindow *window, float timeDelta)
{

	bool speeding = false;
	//timePassed += timeDelta;
	//if (timePassed > 0.15) {
	//	timePassed = 0;
	//}
	// because we use bullet for motion, moveSpeed has to be quiet high for realistic feel
	float moveSpeed = 8;


	// speeding is only allowed if player is not overweight and he has not run for longer than MAX_RUN_TIME
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) && !overWeight && canRun) {
		moveSpeed = 16;
		speeding = true;

		if (runDur <= MAX_RUN_TIME) {
			runDur += timeDelta;
		}
	}

	// player ran too long
	if (runDur > MAX_RUN_TIME) {
		canRun = false;
		breakDur += timeDelta;   // start cool off timer

		// cool off period over?
		if (breakDur >= BREAK_TIME) {
			breakDur = 0;
			runDur = 0;
			canRun = true;
		}
	}

	bool moving = false;

	glm::vec3 dirWorld = glm::normalize(glm::vec3(glm::column(getMatrix(), 2)));

	// player movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) { //  && timePassed == 0
		playerBody->setLinearVelocity(btVector3(dirWorld.x, -1, dirWorld.z) * moveSpeed);
		btTransform trans; 
		playerBody->getMotionState()->getWorldTransform(trans);//->getWorldTransform(trans);

		setLocation(glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
		
		moving = true;
    }
	else if (glfwGetKey(window, 'S')) { // && timePassed == 0
		playerBody->setLinearVelocity(btVector3(-dirWorld.x, -1, -dirWorld.z) * moveSpeed);
		//btTransform trans = playerBody->getWorldTransform();//->getWorldTransform(trans);
		btTransform trans;
		playerBody->getMotionState()->getWorldTransform(trans);//->getWorldTransform(trans);

		setLocation(glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));

		moving = true;
	}
	else {
		playerBody->setLinearVelocity(btVector3(0, 0, 0) * moveSpeed);

		btTransform trans;
		playerBody->getMotionState()->getWorldTransform(trans);//->getWorldTransform(trans);

		setLocation(glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
	}

	if (glfwGetKey(window, 'A')) {
		rotateY(timeDelta * glm::radians(200.f), SceneObject::RIGHT);
    }
	else if (glfwGetKey(window, 'D')) {
		rotateY(timeDelta * glm::radians(-200.f), SceneObject::RIGHT);
    }

	
	//// rotate camera based on mouse movement
	//// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	glm::vec3 camToTarget = camera->getLocation() - getLocation();
	glm::vec3 rightVec = glm::normalize(glm::cross(camToTarget, glm::vec3(0, 1, 0)));
	glm::mat4 rotateYaw = glm::rotate(glm::mat4(), -mouseSensitivity * (float)mouseX, glm::vec3(0, 1, 0));
	glm::mat4 rotatePitch = glm::rotate(glm::mat4(), mouseSensitivity * (float)mouseY, rightVec);

	camToTarget = glm::vec3(rotateYaw * glm::vec4(camToTarget, 0));
	camToTarget = glm::vec3(rotatePitch * glm::vec4(camToTarget, 0));
	camToTarget = camToTarget + getLocation();
	camera->setLocation(camToTarget);

	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window


	//// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;	


	// Handle Carrot Consumption

	// digestion happens faster when moving
	if (speeding && moving) {
		digestDur += timeDelta;	// speeding while moving, fast digestion
	}
	/*
	else if (moving) {
		digestDur += 0.5f * timeDelta;	// only moving
	}
	else {
		digestDur = 0.25f * timeDelta;		// no movement at all, slow digestion
	}*/

	// Digestion Period reached
	if (digestDur > DIGEST_TIME && foodCount > 0) {
		--foodCount; // one carrot less
		digestDur = 0;
		if (foodCount < NEEDED_FOOD) {
			overWeight = false; // if I was overweight, not anymore
		}
	}

}

void Player::handleInputFreeCamera(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 10.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 50.0f;
	}
	
	//////////////////////////
	/// CAMERA MOVEMENT
	//////////////////////////

	// camera movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) {
		camera->translate(camera->getMatrix()[2].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'S')) {
		camera->translate(camera->getMatrix()[2].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'A')) {
		camera->translate(camera->getMatrix()[0].xyz() * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'D')) {
		camera->translate(camera->getMatrix()[0].xyz() * timeDelta * moveSpeed, SceneObject::LEFT);
    }

	if (glfwGetKey(window, 'Q')) {
	    camera->translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
	    camera->translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT); // rotate around local x axis (tilt up/down)
	glm::vec3 location = camera->getLocation();
	camera->translate(-location, SceneObject::LEFT);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::LEFT); // rotate around global y at local position
	camera->translate(location, SceneObject::LEFT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void Player::onScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	scrollY += deltaY;
}

void Player::toggleNavMode()
{
	if (cameraNavMode == FOLLOW_PLAYER) {
		cameraNavMode = FREE_FLY;
	}
	else if (cameraNavMode == FREE_FLY) {
		cameraNavMode = FOLLOW_PLAYER;
	}
}

void Player::handleNavModeChange()
{
	if (cameraNavMode == lastNavMode) {
		return;
	}

	glm::mat4 temp = camera->getMatrix();
	camera->setTransform(lastCamTransform);
	lastCamTransform = temp;

	lastNavMode = cameraNavMode;
}

void Player::eat(Geometry *carrot)
{
	if (!(currentFood != nullptr)) {

		++foodCount;
		if (foodCount > NEEDED_FOOD) {
			overWeight = true;
		}
		currentFood = carrot;
	}
}

bool Player::isFull()
{
	return fullStomach;
}

bool Player::isInBush()
{
	return inBush;
}

void Player::setInBush(bool inBush_)
{
	inBush = inBush_;
}

int Player::getFoodCount()
{
	return foodCount;
}

int Player::getNeededFood()
{
	return NEEDED_FOOD;
}

bool Player::isDefenseActive()
{
	return defenseActive;
}

bool Player::attemptDefenseActivation()
{
	if (!defenseActive && foodCount >= DEFENSE_FOOD_COST) {
		defenseActive = true;
		foodCount -= DEFENSE_FOOD_COST;
		return true;
	}

	return false;
}

glm::mat4 Player::getViewMat()
{
	return viewMat;
}

glm::mat4 Player::getProjMat()
{
	return projMat;
}

btRigidBody *Player::getRigidBody()
{
	return playerBody;
}

void Player::resetPlayer()
{
	foodCount = 0;
	fullStomach = false;
	
	if (currentFood != nullptr) {
		currentFood->setLocation(glm::vec3(300, -300, 300));
		currentFood = nullptr;
	}
}

bool Player::isEating()
{
	return animDur > 0;
}

std::string Player::getFoodReaction()
{
	if (foodCount < NEEDED_FOOD * 0.25) {
		return "sooo hungry!";
	}
	else if (foodCount < NEEDED_FOOD * 0.5) {
		return "gimme more!";
	}
	else if (foodCount < NEEDED_FOOD * 0.75) {
		return "need more carrots";
	}
	else if (foodCount < NEEDED_FOOD) {
		return "just a little bit more";
	}
	else if (foodCount == NEEDED_FOOD) {
		return "Winter may come! I'm prepared";
	}
	else if (foodCount > NEEDED_FOOD) {
		return "augh, sooo full!";
	}
	else {
		return " ";
	}

}

bool Player::isInCave()
{
	return inCave;
}

void Player::setIsInCave(bool inC)
{
	inCave = inC;
}
