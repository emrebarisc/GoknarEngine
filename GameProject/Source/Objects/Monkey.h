#pragma once

#include "Goknar/Core.h"
#include "Goknar/Physics/RigidBody.h"

class StaticMeshComponent;
class MovingTriangleMeshCollisionComponent;

class GOKNAR_API Monkey : public RigidBody
{
public:
	Monkey();

	virtual void BeginGame() override;

protected:
	StaticMeshComponent* staticMeshComponent_;
	MovingTriangleMeshCollisionComponent* collisionComponent_;
private:
};