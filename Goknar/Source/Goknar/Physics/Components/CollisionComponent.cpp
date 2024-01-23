#include "pch.h"

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
}

void CollisionComponent::PreInit()
{
	Component::PreInit();
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