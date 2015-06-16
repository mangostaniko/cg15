#include "physics.h"

bool inBush;
const btRigidBody *toDelete = nullptr;

struct CarrotContact : public btCollisionWorld::ContactResultCallback
{
	CarrotContact() {}

	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* carrotBody,
		int partId0,
		int index0,
		const btCollisionObjectWrapper* playerBody,
		int partId1,
		int index1)
	{
		Player *player = (Player*)playerBody->getCollisionObject()->getUserPointer();
		Geometry *geometry = (Geometry*)carrotBody->getCollisionObject()->getUserPointer();

		carrotBody->getCollisionObject()->setActivationState(0);
		player->eatCarrot(geometry);
		toDelete = (btRigidBody*)carrotBody;

		return 0;
	}
};

struct BushContact : public btCollisionWorld::ContactResultCallback
{
	BushContact() {}

	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* bushBody,
		int partId0,
		int index0,
		const btCollisionObjectWrapper* playerBody,
		int partId1,
		int index1)
	{

		Player *player = (Player*)playerBody->getCollisionObject()->getUserPointer();

		Geometry *geometry = (Geometry*)bushBody->getCollisionObject()->getUserPointer();

		inBush = true;

		return 0;
	}
};

CarrotContact carrotCallback;
BushContact bushCallback;

Physics::Physics(Player *player) : player(player)
{
}

Physics::~Physics()
{
}

void Physics::init()
{
	drawDebug = false;
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	debugDrawerPhysics = new SimpleDebugDrawer();
	dynamicsWorld->setDebugDrawer(debugDrawerPhysics);
}

void Physics::stepSimulation(float deltaT)
{
	inBush = false;
	dynamicsWorld->stepSimulation(btScalar(deltaT));

	for (std::vector<btRigidBody*>::iterator it = carrotsGeo.begin(); it != carrotsGeo.end(); ++it) {
		dynamicsWorld->contactPairTest(*it, player->getRigidBody(), carrotCallback);
	}

	for (std::vector<btRigidBody*>::iterator it = bushesGeo.begin(); it != bushesGeo.end(); ++it) {
		dynamicsWorld->contactPairTest(*it, player->getRigidBody(), bushCallback);
	}



	// TODO DELETE THE CARROT WE ALREADY ATE

	player->setInBush(inBush);
}

void Physics::addTerrainShapeToPhysics()
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *groundShape = new btBoxShape(btVector3(btScalar(100.), btScalar(100.), btScalar(100.)));
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -101, 0));

	groundShape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState *myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
	floor = new btRigidBody(rbInfo);
	floor->setFriction(0);
	floor->setCollisionFlags(floor->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	dynamicsWorld->addRigidBody(floor);

}

void Physics::addTreeSphereToPhysics(Geometry *geometry, btScalar radius)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shape = new btSphereShape(radius);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(geometry->getLocation().x, radius, geometry->getLocation().z));

	shape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, localInertia);
	btRigidBody *body = new btRigidBody(info);
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	dynamicsWorld->addRigidBody(body);
}

void Physics::addFoodSphereToPhysics(Geometry *geometry, btScalar radius)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shape = new btSphereShape(radius);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(geometry->getLocation().x, geometry->getLocation().y, geometry->getLocation().z));

	shape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, localInertia);
	btRigidBody *body = new btRigidBody(info);
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	body->setUserPointer(geometry);

	carrotsGeo.push_back(body);

	dynamicsWorld->addRigidBody(body);
}

void Physics::addBushSphereToPhysics(Geometry *geometry, btScalar radius)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shape = new btSphereShape(radius);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(geometry->getLocation().x, geometry->getLocation().y, geometry->getLocation().z));

	shape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, localInertia);
	btRigidBody *body = new btRigidBody(info);
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	body->setUserPointer(geometry);

	bushesGeo.push_back(body);

	dynamicsWorld->addRigidBody(body);
}


void Physics::debugDrawWorld(bool draw)
{
	drawDebug = draw;
	if (!drawDebug) {
		debugDrawerPhysics->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		dynamicsWorld->setDebugDrawer(debugDrawerPhysics);
		dynamicsWorld->debugDrawWorld();
	}
	else {
		debugDrawerPhysics->setDebugMode(0);
		dynamicsWorld->setDebugDrawer(debugDrawerPhysics);
	}
}

SimpleDebugDrawer* Physics::getPhysicsDebugDrawer()
{
	return debugDrawerPhysics;
}

btDiscreteDynamicsWorld* Physics::getDynamicsWorld()
{
	return dynamicsWorld;
}

void Physics::cleanUp()
{
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (int j = 0; j<collisionShapes.size(); j++) {
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	delete debugDrawerPhysics;
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
	collisionShapes.clear();
}