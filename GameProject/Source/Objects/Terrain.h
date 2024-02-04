#pragma once

#include "Goknar/Core.h"
#include "Goknar/Physics/RigidBody.h"

class StaticMeshComponent;
class NonMovingTriangleMeshCollisionComponent;

class GOKNAR_API Terrain : public RigidBody
{
public:
	Terrain();

protected:
	StaticMeshComponent* staticMeshComponent_;
	NonMovingTriangleMeshCollisionComponent* collisionComponent_;
private:
};