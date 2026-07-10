#include "pch.h"

#include "PhysicsDebugger.h"

#include "Color.h"
#include "Engine.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Components/StaticMeshComponent.h"
#include "Debug/DebugDrawer.h"
#include "Managers/ResourceManager.h"
#include "Materials/Material.h"
#include "Model/StaticMeshInstance.h"
#include "Physics/PhysicsUtils.h"

PhysicsDebugger::PhysicsDebugger()
{
}

PhysicsDebugger::~PhysicsDebugger()
{
	DestroyDebugRoot();
}

void PhysicsDebugger::drawLine(const btVector3& bulletFrom, const btVector3& bulletTo, const btVector3& color)
{
#ifdef GOKNAR_BUILD_DEBUG
	Vector3 from = PhysicsUtils::FromBtVector3ToVector3(bulletFrom);
	Vector3 to = PhysicsUtils::FromBtVector3ToVector3(bulletTo);

	if ((to - from).SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	Colorf debugColor{ color.x(), color.y(), color.z() };
	debugColor.ClampColor(0.f, 1.f);

	DebugDrawer::DrawLine(from, to, debugColor, lineThickness_, -1.f, GetOrCreateDebugRoot());
#endif
}

void PhysicsDebugger::drawContactPoint(
	const btVector3& pointOnB,
	const btVector3& normalOnB,
	btScalar distance,
	int lifeTime,
	const btVector3& color)
{
	(void)distance;
	(void)lifeTime;

	drawLine(pointOnB, pointOnB + normalOnB * contactNormalLength_, color);
}

void PhysicsDebugger::reportErrorWarning(const char* warningString)
{
	if (!warningString || warningString[0] == '\0')
	{
		return;
	}

	GOKNAR_CORE_WARN("Bullet Physics: %s", warningString);
}

void PhysicsDebugger::draw3dText(const btVector3& location, const char* textString)
{
	(void)location;
	(void)textString;
}

void PhysicsDebugger::setDebugMode(int debugMode)
{
	debugMode_ = debugMode;

	if (debugMode_ == btIDebugDraw::DBG_NoDebug)
	{
		clearLines();
	}
}

int PhysicsDebugger::getDebugMode() const
{
	return debugMode_;
}

btIDebugDraw::DefaultColors PhysicsDebugger::getDefaultColors() const
{
	return defaultColors_;
}

void PhysicsDebugger::setDefaultColors(const DefaultColors& colors)
{
	defaultColors_ = colors;
}

void PhysicsDebugger::clearLines()
{
	DestroyDebugRoot();
}

void PhysicsDebugger::flushLines()
{
}

void PhysicsDebugger::SetLineThickness(float thickness)
{
	lineThickness_ = GoknarMath::Max(0.01f, thickness);
}

float PhysicsDebugger::GetLineThickness() const
{
	return lineThickness_;
}

DebugObject* PhysicsDebugger::GetOrCreateDebugRoot()
{
	if (!debugRoot_)
	{
		debugRoot_ = new DebugObject();
		debugRoot_->SetName("__PhysicsDebugger__");
	}

	return debugRoot_;
}

void PhysicsDebugger::DestroyDebugRoot()
{
	if (!debugRoot_)
	{
		return;
	}

	debugRoot_->Destroy();
	debugRoot_ = nullptr;
}
