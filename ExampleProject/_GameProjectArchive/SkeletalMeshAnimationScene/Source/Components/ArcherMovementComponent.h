#ifndef __ARCHERMOVEMENTCOMPONENT_H__
#define __ARCHERMOVEMENTCOMPONENT_H__

#include "Goknar/Components/Component.h"

#include "InterpolatedValue.h"

class Archer;
class Camera;
class SkeletalMeshComponent;
class SkeletalMeshInstance;

class ArcherMovementComponent : public Component
{
public:
	ArcherMovementComponent(Component* parent);

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	void AddMovementVector(const Vector3& movement)
	{
		SetMovementVector(movementVector_.destination + movement);
	}

	void SetMovementVector(const Vector3& movementVector);

	void SetOwnerArcher(Archer* ownerArcher)
	{
		ownerArcher_ = ownerArcher;
	}

protected:

private:
	InterpolatedValue<Vector3> movementVector_;
	InterpolatedValue<float> movementRotation_;

	Archer* ownerArcher_{ nullptr };
	Camera* thirdPersonCamera_{ nullptr };
	SkeletalMeshComponent* archerSkeletalMeshComponent_{ nullptr };
	SkeletalMeshInstance* archerSkeletalMeshInstance_{ nullptr };

	float movementSpeed_;
};


#endif