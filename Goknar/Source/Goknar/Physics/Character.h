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
	~Character();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void Destroy() override;

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void PhysicsTick(float deltaTime) override;

	virtual void UpdateWorldTransformationMatrix() override;


	CapsuleCollisionComponent* GetCharacterCapsuleCollisionComponent() const
	{
		return characterCapsuleCollisionComponent_;
	}

	CharacterMovementComponent* GetCharacterMovementComponent() const
	{
		return characterMovementComponent_;
	}

	SkeletalMeshComponent* GetCharacterSkeletalMeshComponent() const
	{
		return characterSkeletalMeshComponent_;
	}

protected:
	CapsuleCollisionComponent* characterCapsuleCollisionComponent_;
	CharacterMovementComponent* characterMovementComponent_;
	SkeletalMeshComponent* characterSkeletalMeshComponent_;

private:
};

#endif