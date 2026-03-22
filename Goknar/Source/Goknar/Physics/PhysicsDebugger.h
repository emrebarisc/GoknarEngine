#ifndef __PHYSICSDEBUGGER_H__
#define __PHYSICSDEBUGGER_H__

#include "Core.h"
#include "LinearMath/btIDebugDraw.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif

class GOKNAR_API PhysicsDebugger : public btIDebugDraw
{
public:
    PhysicsDebugger();
    ~PhysicsDebugger();

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override {}
    virtual void reportErrorWarning(const char*) override {}
    virtual void draw3dText(const btVector3&, const char*) override {}
    virtual void setDebugMode(int debugMode) override {}
    virtual int getDebugMode() const override { return 0; }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif