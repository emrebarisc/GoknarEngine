#include "pch.h"

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
