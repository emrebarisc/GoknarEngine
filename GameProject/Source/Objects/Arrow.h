#pragma once

#include "StaticMeshObject.h"

class ProjectileMovementComponent;
class SphereCollisionComponent;
class ObjectBase;
class CollisionComponent;

class GOKNAR_API Arrow : public StaticMeshObject
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
	void OnOverlapBegin(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapContinue(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);
	void OnOverlapEnd(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal);

	ProjectileMovementComponent* movementComponent_;
	
	SphereCollisionComponent* overlappingCollisionComponent_;
};