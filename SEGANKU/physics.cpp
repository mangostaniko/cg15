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
		player->eat(geometry);
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
//		Player *player = (Player*)playerBody->getCollisionObject()->getUserPointer();
//		Geometry *geometry = (Geometry*)bushBody->getCollisionObject()->getUserPointer();
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

void Physics::addTerrainShapeToPhysics(Geometry *geometry)
{
	
	int vertStride = sizeof(btVector3);
	int indexStride = 3 * sizeof(int);
	
	std::vector<Vertex> vertices = geometry->getSurface()->getVertices();
	std::vector<GLuint> indices = geometry->getSurface()->getIndices();
	
	const int triangles = vertices.size() / 3;

	/*
	btVector3 *gVertices = new btVector3[vertices.size()];
	int *gIndices = new int[triangles * 3];

	for (int i = 0; i < vertices.size(); ++i) {
		gVertices[i] = btVector3(vertices.at(i).position.x, vertices.at(i).position.y, vertices.at(i).position.z);
	}

	for (int i = 0; i < indices.size(); ++i) {
		gIndices[i] = indices.at(i);
	}

	btTriangleIndexVertexArray *vertArray = new btTriangleIndexVertexArray(triangles, gIndices, indexStride, 
		vertices.size(), (btScalar*) &gVertices[0].x(), vertStride);

	*/

	btTriangleMesh *mTriMesh = new btTriangleMesh();
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		btVector3 v1(vertices.at(i).position.x, vertices.at(i).position.y, vertices.at(i).position.z);
		btVector3 v2(vertices.at(i+1).position.x, vertices.at(i+1).position.y, vertices.at(i+1).position.z);
		btVector3 v3(vertices.at(i+2).position.x, vertices.at(i+2).position.y, vertices.at(i+2).position.z);
		mTriMesh->addTriangle(v1, v2, v3);
		i += 2;
	}

	btTriangleMeshShape *terrain = new btBvhTriangleMeshShape(mTriMesh, false);

	
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, geometry->getLocation().y-1, 0));
	//groundTransform.setFromOpenGLMatrix(glm::value_ptr(geometry->getMatrix()));
	//terrain->calculateLocalInertia(mass, localInertia);
	
	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState *myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, terrain, localInertia);
	floor = new btRigidBody(rbInfo);
	floor->setActivationState(DISABLE_DEACTIVATION);
	floor->setCollisionFlags(floor->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	floor->setFriction(10);

	
	//collisionShapes.push_back(terrain);

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

	//collisionShapes.push_back(shape);

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
	//collisionShapes.push_back(shape);

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
	//collisionShapes.push_back(shape);

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
