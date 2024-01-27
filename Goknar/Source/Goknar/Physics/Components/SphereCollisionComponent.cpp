#include "pch.h"

#include "BulletCollision/CollisionShapes/btSphereShape.h"

#include "SphereCollisionComponent.h"

SphereCollisionComponent::SphereCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{
}

SphereCollisionComponent::SphereCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

SphereCollisionComponent::~SphereCollisionComponent()
{
}

void SphereCollisionComponent::PreInit()
{
	CollisionComponent::PreInit();

	bulletCollisionShape_ = new btSphereShape(radius_);
	bulletCollisionShape_->setLocalScaling(btVector3(worldScaling_.x, worldScaling_.y, worldScaling_.z));
}

void SphereCollisionComponent::Init()
{
	CollisionComponent::Init();

}

void SphereCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();

}

void SphereCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();

}

void SphereCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

}

void SphereCollisionComponent::SetRadius(float radius)
{
	radius_ = radius;
}