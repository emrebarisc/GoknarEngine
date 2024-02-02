#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;
class NonMovingTriangleMeshCollisionComponent;

class GOKNAR_API Terrain : public ObjectBase
{
public:
	Terrain();

protected:
	StaticMeshComponent* staticMeshComponent_;
	NonMovingTriangleMeshCollisionComponent* collisionComponent_;
private:
};