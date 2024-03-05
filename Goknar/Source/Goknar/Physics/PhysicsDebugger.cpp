#include "pch.h"

#include "PhysicsDebugger.h"

#include "Color.h"
#include "Engine.h"
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
}

void PhysicsDebugger::drawLine(const btVector3& bulletFrom, const btVector3& bulletTo, const btVector3& color)
{
	Vector3 from = PhysicsUtils::FromBtVector3ToVector3(bulletFrom);
	Vector3 to = PhysicsUtils::FromBtVector3ToVector3(bulletTo);

	DebugDrawer::DrawLine(from, to, Colorf{ color.x(), color.y(), color.z() });
}
