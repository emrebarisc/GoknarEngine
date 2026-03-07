#include "Characters/BaseCharacter.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Controllers/DefaultCharacterController.h"
#include "Components/DefaultCharacterMovementComponent.h"

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
}

void BaseCharacter::RunForward()
{
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

void BaseCharacter::Jump()
{
}

void BaseCharacter::OnAir()
{
}

void BaseCharacter::OnGround()
{
}

