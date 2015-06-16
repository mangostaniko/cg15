#ifndef SIMPLE_DEBUG_DRAWER_H
#define SIMPLE_DEBUG_DRAWER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <LinearMath/btIDebugDraw.h>
#include <stdio.h>

class SimpleDebugDrawer : public btIDebugDraw
{

	int debugMode;
public:
	SimpleDebugDrawer();
	virtual ~SimpleDebugDrawer();

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

	virtual void	drawSphere(const btVector3& p, btScalar radius, const btVector3& color);

	virtual void	drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);

	virtual void	drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

	virtual void	reportErrorWarning(const char* warningString);

	virtual void	draw3dText(const btVector3& location, const char* textString);

	virtual void	setDebugMode(int debugMode);

	virtual int		getDebugMode() const { return debugMode; }

};

#endif // SIMPLE_DEBUG_DRAWER_H
