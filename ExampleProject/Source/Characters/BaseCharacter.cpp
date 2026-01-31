#include "Characters/BaseCharacter.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Controllers/FreeCameraController.h"

BaseCharacter::BaseCharacter() : 
	Character()
{
	SetIsTickable(false);

	SetCollisionGroup((CollisionGroup)((int)CollisionGroup::Custom0 | (int)CollisionGroup::Custom1 | (int)CollisionGroup::Character));
	SetCollisionMask((CollisionMask)((int)CollisionMask::BlockAndOverlapAll));
}

BaseCharacter::~BaseCharacter()
{
	controller_->Destroy();
}

void BaseCharacter::BeginGame()
{
	Character::BeginGame();
}

void BaseCharacter::Tick(float deltaTime)
{
	Character::Tick(deltaTime);
}

void BaseCharacter::Idle()
{
	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Idle");
}

void BaseCharacter::RunForward()
{
	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("RunForward");
}

void BaseCharacter::RunBackward()
{
}

void BaseCharacter::RunRight()
{
}

void BaseCharacter::RunLeft()
{
}

void BaseCharacter::WalkForward()
{
	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("WalkForward");
}

void BaseCharacter::WalkBackward()
{
}

void BaseCharacter::WalkRight()
{
}

void BaseCharacter::WalkLeft()
{
}

void BaseCharacter::Die()
{
}

void BaseCharacter::OnAttackStarted()
{
	isAttacking_ = true;
}

void BaseCharacter::OnAttackFinished()
{
	isAttacking_ = false;
	Idle();
}
