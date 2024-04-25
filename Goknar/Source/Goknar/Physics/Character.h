#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "Core.h"

#include "Goknar/Physics/OverlappingPhysicsObject.h"

class CapsuleCollisionComponent;
class CharacterMovementComponent;
class SkeletalMeshComponent;

class GOKNAR_API Character : public OverlappingPhysicsObject
{
public:
	Character();
	virtual ~Character();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void Destroy() override;

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void PhysicsTick(float deltaTime) override;

	virtual void UpdateWorldTransformationMatrix() override;

	CapsuleCollisionComponent* GetCapsuleCollisionComponent() const
	{
		return capsuleCollisionComponent_;
	}

	CharacterMovementComponent* GetMovementComponent() const
	{
		return movementComponent_;
	}

	SkeletalMeshComponent* GetSkeletalMeshComponent() const
	{
		return skeletalMeshComponent_;
	}

protected:
	CapsuleCollisionComponent* capsuleCollisionComponent_{ nullptr };
	CharacterMovementComponent* movementComponent_{ nullptr };
	SkeletalMeshComponent* skeletalMeshComponent_{ nullptr };

private:
};

#endif