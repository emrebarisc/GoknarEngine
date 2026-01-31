#pragma once

#include "Physics/RigidBody.h"

class SphereCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API PhysicsSphere : public RigidBody
{
public:
	PhysicsSphere();

	virtual void BeginGame() override;
protected:

private:
	SphereCollisionComponent* sphereCollisionComponent_;
	StaticMeshComponent* staticMeshComponent_;
};