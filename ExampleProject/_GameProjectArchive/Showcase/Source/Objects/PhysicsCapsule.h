#pragma once

#include "Physics/RigidBody.h"

class CapsuleCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API PhysicsCapsule : public RigidBody
{
public:
	PhysicsCapsule();

	virtual void BeginGame() override;

	StaticMeshComponent* GetStaticMeshComponent() const
	{
		return staticMeshComponent_;
	}
protected:

private:
	CapsuleCollisionComponent* capsuleCollisionComponent_;
	StaticMeshComponent* staticMeshComponent_;
};