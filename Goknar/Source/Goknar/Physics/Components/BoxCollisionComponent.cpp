#include "pch.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include "BoxCollisionComponent.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Physics/PhysicsUtils.h"

BoxCollisionComponent::BoxCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{

}

BoxCollisionComponent::BoxCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

BoxCollisionComponent::~BoxCollisionComponent()
{
}

void BoxCollisionComponent::PreInit()
{
	bulletCollisionShape_ = new btBoxShape(PhysicsUtils::FromVector3ToBtVector3(halfSize_));
	
	CollisionComponent::PreInit();
}

void BoxCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void BoxCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void BoxCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void BoxCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}

void BoxCollisionComponent::SetHalfSize(const Vector3& halfSize)
{
	if (GetIsInitialized())
	{
		GOKNAR_CORE_WARN("Trying to set half size after initialization.");
		return;
	}

	halfSize_ = halfSize;
}