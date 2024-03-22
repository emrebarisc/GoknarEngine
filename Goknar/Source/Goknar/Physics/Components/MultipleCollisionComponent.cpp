#include "pch.h"

#include "BulletCollision/CollisionShapes/btCompoundShape.h"

#include "MultipleCollisionComponent.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Physics/PhysicsUtils.h"
	
MultipleCollisionComponent::MultipleCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{
}

MultipleCollisionComponent::MultipleCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

MultipleCollisionComponent::~MultipleCollisionComponent()
{
}

void MultipleCollisionComponent::PreInit()
{
	bulletCompoundShape_ = new btCompoundShape();
	bulletCollisionShape_ = bulletCompoundShape_;
	
	CollisionComponent::PreInit();
}

void MultipleCollisionComponent::Init()
{
	CollisionComponent::Init();
	
	std::vector<CollisionComponent*>::const_iterator subCollisionComponentIterator = subCollisionComponents_->cbegin();

	while(subCollisionComponentIterator != subCollisionComponents_->cend())
	{
		AddCollisionComponentInner(*subCollisionComponentIterator);

		++subCollisionComponentIterator;
	}

	delete subCollisionComponents_;
}

void MultipleCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void MultipleCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void MultipleCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);
}

void MultipleCollisionComponent::AddCollisionComponent(CollisionComponent* collisionComponent)
{
	if(!GetIsInitialized())
	{
		subCollisionComponents_->push_back(collisionComponent);
		return;
	}

	AddCollisionComponentInner(collisionComponent);
}

void MultipleCollisionComponent::AddCollisionComponentInner(CollisionComponent* collisionComponent)
{
	btCollisionShape* bulletCollisionComponent = collisionComponent->GetBulletCollisionShape();

	btTransform bulletTransform = PhysicsUtils::GetBulletTransform(collisionComponent->GetRelativeRotation(), collisionComponent->GetRelativePosition());

	bulletCompoundShape_->addChildShape(bulletTransform, bulletCollisionComponent);
}