#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;
class StaticTriangleMeshCollisionComponent;

class GOKNAR_API Terrain : public ObjectBase
{
public:
	Terrain();

protected:
	StaticMeshComponent* staticMeshComponent_;
	StaticTriangleMeshCollisionComponent* collisionComponent_;
private:
};