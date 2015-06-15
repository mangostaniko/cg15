#ifndef PHYSICS_H
#define PHYSICS_H

#include <btBulletDynamicsCommon.h>
#include <BulletCollision\CollisionDispatch\btCollisionWorld.h>
#include "simpledebugdrawer.h"
#include "geometry.h"
#include "player.h"

class Physics
{
public:
	Physics(Player *player);
	~Physics();

	void init();
	void cleanUp();

	void stepSimulation(float deltaT);
	void debugDrawWorld(bool draw);
	void addFoodSphereToPhysics(Geometry *geometry, btScalar radius);
	void addBushSphereToPhysics(Geometry *geometry, btScalar radius);
	void addTreeSphereToPhysics(Geometry *geometry, btScalar radius);
	void addTerrainShapeToPhysics();

	SimpleDebugDrawer *getPhysicsDebugDrawer();
	btDiscreteDynamicsWorld *getDynamicsWorld();

private:

	Player *player;

	btRigidBody *floor;

	bool drawDebug;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *dispatcher;
	btBroadphaseInterface * overlappingPairCache;
	btSequentialImpulseConstraintSolver *solver;
	
	btDiscreteDynamicsWorld *dynamicsWorld;

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	
	SimpleDebugDrawer *debugDrawerPhysics;

	std::vector<btRigidBody*> carrotsGeo;
	std::vector<btRigidBody*> bushesGeo;
};

#endif// PHYSICS_H

