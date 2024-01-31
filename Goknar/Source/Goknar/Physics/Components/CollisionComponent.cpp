#include "pch.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include "CollisionComponent.h"

CollisionComponent::CollisionComponent(Component* parent) :
	Component(parent)
{
}

CollisionComponent::CollisionComponent(ObjectBase* parentObjectBase) :
	Component(parentObjectBase)
{
}

CollisionComponent::~CollisionComponent()
{
	delete bulletCollisionShape_;
}

void CollisionComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	if (!GetIsInitialized())
	{
		return;
	}

	bulletCollisionShape_->setLocalScaling(btVector3(worldScaling_.x, worldScaling_.y, worldScaling_.z));
}

void CollisionComponent::PreInit()
{
	Component::PreInit();
	
	bulletCollisionShape_->setLocalScaling(btVector3(worldScaling_.x, worldScaling_.y, worldScaling_.z));
}

void CollisionComponent::Init()
{
	Component::Init();
}

void CollisionComponent::PostInit()
{
	Component::PostInit();
}
	
void CollisionComponent::BeginGame()
{
	Component::BeginGame();
}

void CollisionComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
}