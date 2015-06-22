#include "physics.h"

bool inBush, inCave;
btRigidBody *toDelete = nullptr;
btDiscreteDynamicsWorld *physicWorld;

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

		if (player->isEating()) {
			return 0;
		}
		carrotBody->getCollisionObject()->setActivationState(DISABLE_SIMULATION);

		//removeCollisionObject(const_cast<btCollisionObject*>(colObj0Wrap->getCollisionObject()));
		player->eat(geometry);
		physicWorld->removeCollisionObject(const_cast<btCollisionObject*>(carrotBody->getCollisionObject()));
		toDelete = (btRigidBody*)carrotBody->getCollisionObject();

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

		inBush = true;

		return 0;
	}
};

struct CaveAreaContact : public btCollisionWorld::ContactResultCallback
{
	CaveAreaContact() {}

	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* caveBody,
		int partId0,
		int index0,
		const btCollisionObjectWrapper* playerBody,
		int partId1,
		int index1)
	{

		inBush = true;

		return 0;
	}
};

struct CaveContact : public btCollisionWorld::ContactResultCallback
{
	CaveContact() {}

	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* caveBody,
		int partId0,
		int index0,
		const btCollisionObjectWrapper* playerBody,
		int partId1,
		int index1)
	{

//		inCave = true;

		return 0;
	}
};
CarrotContact carrotCallback;
BushContact bushCallback;
CaveAreaContact caveAreaCallback;
CaveContact caveCallback;

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
	physicWorld = dynamicsWorld;
}

void Physics::stepSimulation(float deltaT)
{
	inBush = false;
	inCave = false;
	dynamicsWorld->stepSimulation(btScalar(deltaT));

	for (std::vector<btRigidBody*>::iterator it = carrotsGeo.begin(); it != carrotsGeo.end(); ++it) {
		dynamicsWorld->contactPairTest(*it, player->getRigidBody(), carrotCallback);
	}

	for (std::vector<btRigidBody*>::iterator it = bushesGeo.begin(); it != bushesGeo.end(); ++it) {
		dynamicsWorld->contactPairTest(*it, player->getRigidBody(), bushCallback);
	}

	dynamicsWorld->contactPairTest(caveArea, player->getRigidBody(), caveAreaCallback);

	dynamicsWorld->contactPairTest(cave, player->getRigidBody(), caveCallback);

	// DELETE THE CARROT WE ALREADY ATE
	
	if (toDelete != nullptr) {
		carrotsGeo.erase(std::find(carrotsGeo.begin(), carrotsGeo.end(), toDelete));
		deletedBodies.push_back(toDelete);
		toDelete = nullptr;
	}
	
	player->setInBush(inBush);
	player->setIsInCave(inCave);
}

void Physics::addTerrainShapeToPhysics(Geometry *geometry)
{
	int vertStride = sizeof(btVector3);
	int indexStride = 3 * sizeof(int);
	
	std::vector<Vertex> vertices = geometry->getSurface()->getVertices();
	std::vector<GLuint> indices = geometry->getSurface()->getIndices();
	
	const int triangles = vertices.size() / 3;

	btTriangleMesh *mTriMesh = new btTriangleMesh();
	for (unsigned int i = 0; i < vertices.size(); i += 3) {
		btVector3 v1(vertices.at(i).position.x, vertices.at(i).position.y, vertices.at(i).position.z);
		btVector3 v2(vertices.at(i+1).position.x, vertices.at(i+1).position.y, vertices.at(i+1).position.z);
		btVector3 v3(vertices.at(i+2).position.x, vertices.at(i+2).position.y, vertices.at(i+2).position.z);
		mTriMesh->addTriangle(v1, v2, v3);
	}

	btTriangleMeshShape *terrain = new btBvhTriangleMeshShape(mTriMesh, true);
	
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, geometry->getLocation().y-1, 0));
	
	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState *myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, terrain, localInertia);
	floor = new btRigidBody(rbInfo);
	floor->setActivationState(DISABLE_DEACTIVATION);
	floor->setCollisionFlags(floor->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	floor->setFriction(10);

	dynamicsWorld->addRigidBody(floor);

}

void Physics::addTreeCylinderToPhysics(Geometry *geometry, btScalar radius)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shape = new btCylinderShape(btVector3(radius, 10, radius));
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

void Physics::setupCaveObjects(Geometry *geometry)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shapeArea = new btSphereShape(btScalar(3.));
	btCollisionShape *shapeCave = new btSphereShape(btScalar(1.5));

	// 1.5, 1.5, 2
	// 4.5, 4, 4.5
	btCollisionShape *shapeEnd = new btBoxShape(btVector3(2, 2, 0.15));
	btCollisionShape *shapeSide1 = new btBoxShape(btVector3(0.15, 2, 2.25));
	btCollisionShape *shapeSide2 = new btBoxShape(btVector3(0.15, 2, 2.25));

	// setup Cave Area -> no collision
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(geometry->getLocation().x, geometry->getLocation().y, geometry->getLocation().z));
	shapeArea->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shapeArea, localInertia);
	caveArea = new btRigidBody(info);
	caveArea->setActivationState(DISABLE_DEACTIVATION);
	caveArea->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	// setup inside cave -> no collision -> zone to be in for win
	shapeCave->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState2 = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info2(mass, motionState2, shapeCave, localInertia);
	cave = new btRigidBody(info2);
	cave->setActivationState(DISABLE_DEACTIVATION);
	cave->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	// setup cave walls -> collision -> back wall
	transform.setOrigin(btVector3(geometry->getLocation().x, geometry->getLocation().y, geometry->getLocation().z-2.5));
	shapeEnd->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *stateEnd = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo infoEnd(mass, stateEnd, shapeEnd, localInertia);
	btRigidBody *caveBack = new btRigidBody(infoEnd);
	caveBack->setActivationState(DISABLE_DEACTIVATION);
	caveBack->setCollisionFlags(caveBack->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	transform.setOrigin(btVector3(geometry->getLocation().x+2, geometry->getLocation().y, geometry->getLocation().z));
	shapeSide1->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *stateSide1 = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo infoSide1(mass, stateSide1, shapeSide1, localInertia);
	btRigidBody *caveSide1 = new btRigidBody(infoSide1);
	caveSide1->setActivationState(DISABLE_DEACTIVATION);
	caveSide1->setCollisionFlags(caveSide1->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	transform.setOrigin(btVector3(geometry->getLocation().x - 2, geometry->getLocation().y, geometry->getLocation().z));
	shapeSide2->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *stateSide2 = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo infoSide2(mass, stateSide2, shapeSide2, localInertia);
	btRigidBody *caveSide2 = new btRigidBody(infoSide2);
	caveSide2->setActivationState(DISABLE_DEACTIVATION);
	caveSide2->setCollisionFlags(caveSide2->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	dynamicsWorld->addRigidBody(caveSide1);
	dynamicsWorld->addRigidBody(caveSide2);
	dynamicsWorld->addRigidBody(caveBack);
	dynamicsWorld->addRigidBody(caveArea);
	dynamicsWorld->addRigidBody(cave);
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
