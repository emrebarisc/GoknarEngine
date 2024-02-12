#pragma once

#include "Physics/OverlappingPhysicsObject.h"

class ProjectileMovementComponent;
class BoxCollisionComponent;
class ObjectBase;
class CollisionComponent;
class StaticMeshComponent;

class GOKNAR_API Arrow : public OverlappingPhysicsObject
{
public:
	Arrow();

	virtual void BeginGame() override;

	void Shoot();

	ProjectileMovementComponent* GetMovementComponent() const
	{
		return movementComponent_;
	}

protected:

private:
	void OnOverlapBegin(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapContinue(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapEnd(PhysicsObject* otherObject, CollisionComponent* otherComponent);

	ProjectileMovementComponent* movementComponent_;

	BoxCollisionComponent* overlappingCollisionComponent_;
	StaticMeshComponent* staticMeshComponent_;
};