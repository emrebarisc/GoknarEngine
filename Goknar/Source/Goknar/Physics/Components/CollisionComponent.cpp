#include "pch.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "CollisionComponent.h"
#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

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

void CollisionComponent::SetIsOverlapping(bool isOverlapping)
{
	GOKNAR_CORE_ASSERT(!GetIsInitialized());

	isOverlapping_ = isOverlapping;
}

void CollisionComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	if (!GetIsInitialized())
	{
		return;
	}
	
	UpdateTransformation();
}

void CollisionComponent::UpdateTransformation()
{
	bulletCollisionShape_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(worldScaling_));
	
	if(isOverlapping_)
	{
		btTransform collisionObjectTransform;
		bulletCollisionObject_->setWorldTransform(btTransform(
				PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation_), 
				PhysicsUtils::FromVector3ToBtVector3(worldPosition_))
		);
		
		// bulletCollisionObject_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(worldScaling_));
	}
}

void CollisionComponent::PreInit()
{
	Component::PreInit();

	bulletCollisionShape_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(worldScaling_));

	if(isOverlapping_)
	{
		GOKNAR_CORE_ASSERT(
			dynamic_cast<RigidBody*>(GetOwner()) == nullptr || 
			(dynamic_cast<RigidBody*>(GetOwner()) != nullptr && 
			GetOwner()->GetRootComponent() != this), 
			"Overlapping component cannot be the root component of a RigidBody!");
			
		bulletCollisionObject_ = new btPairCachingGhostObject();
		bulletCollisionObject_->setCollisionShape(bulletCollisionShape_);
		bulletCollisionObject_->setWorldTransform(
			btTransform(
				PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation_), 
				PhysicsUtils::FromVector3ToBtVector3(worldPosition_))
			);

		engine->GetPhysicsWorld()->AddOverlappingCollisionComponent(this);
	}
	else
	{
		GOKNAR_CORE_ASSERT(
			dynamic_cast<RigidBody*>(GetOwner()) &&
			GetOwner()->GetRootComponent() == this,
			"Collision component's owner must be a RigidBody and this must be root component of it");

		bulletCollisionObject_ = dynamic_cast<RigidBody*>(GetOwner())->GetBulletRigidBody();
	}
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