#ifndef PHYSICS_H
#define PHYSICS_H

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include "simpledebugdrawer.h"
#include "geometry.h"
#include "player.h"

class Physics
{
public:
	Physics(Player *player);
	~Physics();

	/**
	* @brief initialises physics world
	*/
	void init();
	
	/**
	* @brief clean up physics world
	*/
	void cleanUp();

	/**
	* @brief step the physics simulation ahead
	* @param deltaT deltaTime
	*/
	void stepSimulation(float deltaT);

	/**
	* @brief use debugDrawer -> NOT IMPLEMENTED
	* @param draw true to draw else false
	*/
	void debugDrawWorld(bool draw);

	/**
	* @brief add a new CollisionBody to the Physics World, no Collision Handling set
	* @param geometry the geometry object that is to be a collision object (-> use for Carrots and other food)
	* @param radius radius for the Sphere Collision Object
	*/
	void addFoodSphereToPhysics(Geometry *geometry, btScalar radius);
	
	/**
	* @brief add a new CollisionBody to the Physics World, no Collision Handling set
	* @param geometry the geometry object that is to be a collision object (-> use for Bushes and other objects for hiding)
	* @param radius radius for the Sphere Collision Object
	*/
	void addBushSphereToPhysics(Geometry *geometry, btScalar radius);

	/**
	* @brief add a new CollisionBody to the Physics World, set Collision Handling for static object
	* @param geometry the geometry object that is to be a collision object (-> use for Tree and other static objects)
	* @param radius radius for the Sphere Collision Object
	*/
	void addTreeCylinderToPhysics(Geometry *geometry, btScalar radius);

	/**
	* @brief add the TerrainShape to the physics world, for now btCollisionBox is made, later should be made with mesh
	*/
	void addTerrainShapeToPhysics(Geometry *geometry);

	/**
	* @brief add a new CollisionBody to the Physics World, no Collision Handling set
	* @return the debugDrawer
	*/
	SimpleDebugDrawer *getPhysicsDebugDrawer();

	/**
	* @brief 
	* @return the physics world
	*/
	btDiscreteDynamicsWorld *getDynamicsWorld();

private:

	Player *player;

	btRigidBody *floor;

	bool drawDebug;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *dispatcher;
	btBroadphaseInterface *overlappingPairCache;
	btSequentialImpulseConstraintSolver *solver;
	
	btDiscreteDynamicsWorld *dynamicsWorld;

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	
	SimpleDebugDrawer *debugDrawerPhysics;

	std::vector<btRigidBody*> carrotsGeo;
	std::vector<btRigidBody*> bushesGeo;
	std::vector<btRigidBody*> deletedBodies;
};

#endif// PHYSICS_H

