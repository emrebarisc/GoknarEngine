#include "pch.h"

#include "CapsuleCollisionComponent.h"

#include "GoknarAssert.h"
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

Component* CapsuleCollisionComponent::Clone() const
{
	CapsuleCollisionComponent* clonedComponent = new CapsuleCollisionComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);
	clonedComponent->SetCollisionGroup(GetCollisionGroup());
	clonedComponent->SetCollisionMask(GetCollisionMask());
	clonedComponent->radius_ = radius_;
	clonedComponent->height_ = height_;

	return clonedComponent;
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
