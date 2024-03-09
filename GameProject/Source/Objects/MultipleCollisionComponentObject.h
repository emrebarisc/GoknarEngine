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