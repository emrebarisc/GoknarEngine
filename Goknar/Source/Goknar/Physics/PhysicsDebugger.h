#ifndef __PHYSICSDEBUGGER_H__
#define __PHYSICSDEBUGGER_H__

#include "Core.h"

#include "LinearMath/btIDebugDraw.h"

class GOKNAR_API PhysicsDebugger : public btIDebugDraw
{
public:
	PhysicsDebugger();
	~PhysicsDebugger();

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {}

	virtual void reportErrorWarning(const char* warningString) override {}

	virtual void draw3dText(const btVector3& location, const char* textString) override {}

	virtual void setDebugMode(int debugMode) override {}
protected:

private:

};


#endif
