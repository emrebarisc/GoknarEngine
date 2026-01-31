#pragma once

#include "Physics/RigidBody.h"

class BoxCollisionComponent;
class SphereCollisionComponent;
class MultipleCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API MultipleCollisionComponentObject : public RigidBody
{
public:
	MultipleCollisionComponentObject();

	virtual void BeginGame() override;

	BoxCollisionComponent* GetBoxCollisionComponent() const
	{
		return boxCollisionComponent_;
	}

	SphereCollisionComponent* GetSphereCollisionComponent1() const
	{
		return sphereCollisionComponent1_;
	}

	SphereCollisionComponent* GetSphereCollisionComponent2() const
	{
		return sphereCollisionComponent2_;
	}

protected:

private:
	MultipleCollisionComponent* multipleCollisionComponent_;
	StaticMeshComponent* unitSphereStaticMeshComponent1_;
	StaticMeshComponent* unitSphereStaticMeshComponent2_;
	StaticMeshComponent* boxStaticMeshComponent_;
	BoxCollisionComponent* boxCollisionComponent_;
	SphereCollisionComponent* sphereCollisionComponent1_;
	SphereCollisionComponent* sphereCollisionComponent2_;
};