#include "pch.h"

#include "Character.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Physics/PhysicsTypes.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/CharacterMovementComponent.h"

Character::Character() : OverlappingPhysicsObject()
{
    capsuleCollisionComponent_ = AddSubComponent<CapsuleCollisionComponent>();
    skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();

    SetCollisionGroup(CollisionGroup::Character);
    SetCollisionMask(CollisionMask::BlockAllExceptCharacter);
    SetCollisionFlag(CollisionFlag::CharacterObject);
}

Character::~Character()
{

}

void Character::PreInit()
{
    OverlappingPhysicsObject::PreInit();

    if (!movementComponent_)
    {
        movementComponent_ = AddSubComponent<CharacterMovementComponent>();
    }
    movementComponent_->SetCollisionComponent(capsuleCollisionComponent_);
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