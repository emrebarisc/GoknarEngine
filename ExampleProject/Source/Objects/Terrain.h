#pragma once

#include "Goknar/Core.h"
#include "Goknar/Physics/RigidBody.h"

class NonMovingTriangleMeshCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API Terrain : public RigidBody
{
public:
	Terrain();

	NonMovingTriangleMeshCollisionComponent* GetTerrainCollisionComponent() const
	{
		return collisionComponent_;
	}

	StaticMeshComponent* GetTerrainStaticMeshComponent() const
	{
		return staticMeshComponent_;
	}

protected:
	StaticMeshComponent* staticMeshComponent_{ nullptr };
	NonMovingTriangleMeshCollisionComponent* collisionComponent_{ nullptr };
};
