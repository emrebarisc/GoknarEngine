#include "pch.h"

#include "HeightMapCollisionComponent.h"

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "GoknarAssert.h"
#include "ObjectBase.h"
#include "Contents/Image.h"
#include "Model/StaticMesh.h"
#include "Physics/PhysicsUtils.h"

HeightMapCollisionComponent::HeightMapCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{

}

HeightMapCollisionComponent::HeightMapCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

HeightMapCollisionComponent::~HeightMapCollisionComponent()
{
}

void HeightMapCollisionComponent::PreInit()
{
	CollisionComponent::PreInit();

	bulletCollisionShape_ = new btHeightfieldTerrainShape(heightStickWidth_, heightStickLength_, heightMapImage_->GetBuffer(), maxHeight_, 2, true, false);
}

void HeightMapCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void HeightMapCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void HeightMapCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void HeightMapCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);
}

Component* HeightMapCollisionComponent::Clone() const
{
	HeightMapCollisionComponent* clonedComponent = new HeightMapCollisionComponent(static_cast<Component*>(nullptr));
	CopyValuesTo(clonedComponent);
	clonedComponent->SetCollisionGroup(GetCollisionGroup());
	clonedComponent->SetCollisionMask(GetCollisionMask());
	clonedComponent->heightMapImage_ = heightMapImage_;
	clonedComponent->heightStickWidth_ = heightStickWidth_;
	clonedComponent->heightStickLength_ = heightStickLength_;
	clonedComponent->minHeight_ = minHeight_;
	clonedComponent->maxHeight_ = maxHeight_;
	clonedComponent->width_ = width_;
	clonedComponent->length_ = length_; 
	clonedComponent->heightScale_ = heightScale_;

	return clonedComponent;
}
