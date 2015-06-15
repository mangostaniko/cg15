#include "physics.h"


Physics::Physics()
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

void Physics::stepSimulation(float deltaT)
{
	dynamicsWorld->stepSimulation(btScalar(deltaT));
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
	btRigidBody *body = new btRigidBody(rbInfo);
	body->setFriction(0);

	dynamicsWorld->addRigidBody(body);
}

void Physics::addSphereShapeToPhysics(const Geometry &geometry, btScalar radius)
{
	btScalar mass(0.);
	btVector3 localInertia(0, 0, 0);

	btCollisionShape *shape = new btSphereShape(radius);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(geometry.getLocation().x, radius, geometry.getLocation().z));

	shape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape, localInertia);
	btRigidBody *body = new btRigidBody(info);
	body->setActivationState(DISABLE_DEACTIVATION);
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
