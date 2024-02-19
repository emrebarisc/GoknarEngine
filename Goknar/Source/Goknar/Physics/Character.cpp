#include "pch.h"

#include "Character.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/CharacterMovementComponent.h"

Character::Character() : OverlappingPhysicsObject()
{
	characterCapsuleCollisionComponent_ = AddSubComponent<CapsuleCollisionComponent>();
	characterMovementComponent_ = AddSubComponent<CharacterMovementComponent>();
    characterSkeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
}

Character::~Character()
{

}

void Character::PreInit()
{
    OverlappingPhysicsObject::PreInit();
}

void Character::Init()
{
    OverlappingPhysicsObject::Init();
}

void Character::PostInit()
{
    OverlappingPhysicsObject::PostInit();
}

void Character::Destroy()
{
    OverlappingPhysicsObject::Destroy();
}

void Character::BeginGame()
{
    OverlappingPhysicsObject::BeginGame();
}

void Character::Tick(float deltaTime)
{
    OverlappingPhysicsObject::Tick(deltaTime);
}

void Character::PhysicsTick(float deltaTime)
{
	OverlappingPhysicsObject::PhysicsTick(deltaTime);
}

void Character::UpdateWorldTransformationMatrix()
{
    OverlappingPhysicsObject::UpdateWorldTransformationMatrix();
}