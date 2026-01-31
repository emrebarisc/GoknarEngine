#pragma once

#include "Physics/RigidBody.h"

class CollisionComponent;
class SphereCollisionComponent;
class StaticMeshComponent;

class GOKNAR_API CannonBall : public RigidBody
{
public:
	CannonBall();

	virtual void BeginGame() override;
protected:
	void OnOverlapBegin(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapContinue(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapEnd(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);

private:
	SphereCollisionComponent* sphereCollisionComponent_;
	SphereCollisionComponent* overlappingCollisionComponent_;
	StaticMeshComponent* staticMeshComponent_;
};