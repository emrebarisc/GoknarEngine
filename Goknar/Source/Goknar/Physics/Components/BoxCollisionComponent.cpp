#include "pch.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include "BoxCollisionComponent.h"
#include "ObjectBase.h"

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
	delete bulletCollisionShape_;
}

void BoxCollisionComponent::PreInit()
{
	CollisionComponent::PreInit();

	bulletCollisionShape_ = new btBoxShape(btVector3(halfSize_.x, halfSize_.y, halfSize_.z));
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