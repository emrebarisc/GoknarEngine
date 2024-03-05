#include "pch.h"

#include "CapsuleCollisionComponent.h"

#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

CapsuleCollisionComponent::CapsuleCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{
}

CapsuleCollisionComponent::CapsuleCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

CapsuleCollisionComponent::~CapsuleCollisionComponent()
{
}

void CapsuleCollisionComponent::PreInit()
{
	bulletCollisionShape_ = new btCapsuleShapeZ(radius_, height_);

	CollisionComponent::PreInit();
}

void CapsuleCollisionComponent::Init()
{
	CollisionComponent::Init();

}

void CapsuleCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();

}

void CapsuleCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();

}

void CapsuleCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

}

void CapsuleCollisionComponent::SetRadius(float radius)
{
	GOKNAR_CORE_CHECK(!GetIsInitialized(), "Trying to set radius after initialization. Use scaling instead.");
	radius_ = radius;
}

void CapsuleCollisionComponent::SetHeight(float height)
{
	GOKNAR_CORE_CHECK(!GetIsInitialized(), "Trying to set height after initialization. Use scaling instead.");
	height_ = height;
}