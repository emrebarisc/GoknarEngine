#pragma once

#include "Physics/RigidBody.h"

class BoxCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API PhysicsBox : public RigidBody
{
public:
	PhysicsBox();

	virtual void BeginGame() override;
protected:

private:
	BoxCollisionComponent* boxCollisionComponent_;
	StaticMeshComponent* staticMeshComponent_;
};