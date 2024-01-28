#ifndef __PHYSICSARCHERMOVEMENTCOMPONENT_H__
#define __PHYSICSARCHERMOVEMENTCOMPONENT_H__

#include "Goknar/Components/Component.h"

#include "InterpolatedValue.h"

class PhysicsArcher;
class Camera;
class SkeletalMeshComponent;
class SkeletalMeshInstance;

class PhysicsArcherMovementComponent : public Component
{
public:
	PhysicsArcherMovementComponent(Component* parent);

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	void AddMovementVector(const Vector3& movement)
	{
		SetMovementVector(movementVector_.destination + movement);
	}

	void SetMovementVector(const Vector3& movementVector);

	void SetOwnerPhysicsArcher(PhysicsArcher* ownerPhysicsArcher)
	{
		ownerPhysicsArcher_ = ownerPhysicsArcher;
	}

protected:

private:
	InterpolatedValue<Vector3> movementVector_;
	InterpolatedValue<float> movementRotation_;

	PhysicsArcher* ownerPhysicsArcher_{ nullptr };
	Camera* thirdPersonCamera_{ nullptr };
	SkeletalMeshComponent* archerSkeletalMeshComponent_{ nullptr };
	SkeletalMeshInstance* archerSkeletalMeshInstance_{ nullptr };

	float movementSpeed_;
};


#endif