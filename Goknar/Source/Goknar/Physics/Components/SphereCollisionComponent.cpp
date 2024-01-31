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
	bulletCollisionShape_ = new btSphereShape(radius_);
	
	CollisionComponent::PreInit();
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