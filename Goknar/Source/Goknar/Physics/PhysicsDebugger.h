#ifndef __PHYSICSDEBUGGER_H__
#define __PHYSICSDEBUGGER_H__

#include "Core.h"
#include "LinearMath/btIDebugDraw.h"

class DebugObject;

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
    virtual void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
    virtual void reportErrorWarning(const char* warningString) override;
    virtual void draw3dText(const btVector3& location, const char* textString) override;
    virtual void setDebugMode(int debugMode) override;
    virtual int getDebugMode() const override;
    virtual DefaultColors getDefaultColors() const override;
    virtual void setDefaultColors(const DefaultColors& colors) override;
    virtual void clearLines() override;
    virtual void flushLines() override;

    void SetLineThickness(float thickness);
    float GetLineThickness() const;

private:
    DebugObject* GetOrCreateDebugRoot();
    void DestroyDebugRoot();

    DefaultColors defaultColors_{};
    DebugObject* debugRoot_{ nullptr };
    int debugMode_{ btIDebugDraw::DBG_NoDebug };
    float lineThickness_{ 1.f };
    float contactNormalLength_{ 0.35f };
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
