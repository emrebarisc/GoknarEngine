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
	bulletCollisionShape_ = new btCapsuleShape(radius_, height_);

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
	radius_ = radius;

	if(!GetIsInitialized())
	{
		return;
	}

	// bulletCollisionShape_->setRadius(radius_);
}

void CapsuleCollisionComponent::SetHeight(float height)
{
	height_ = height;

	if(!GetIsInitialized())
	{
		return;
	}

	// bulletCollisionShape_->setRadius(radius_);
}