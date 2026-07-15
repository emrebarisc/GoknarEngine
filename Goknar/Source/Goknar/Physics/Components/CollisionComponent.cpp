#include "pch.h"

#include "CollisionComponent.h"
#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"

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
	
	UpdateTransformation();
}

void CollisionComponent::UpdateTransformation()
{
	if (!bulletCollisionShape_)
	{
		return;
	}

	bulletCollisionShape_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(worldScaling_));

	RigidBody* ownerRigidBody = dynamic_cast<RigidBody*>(owner_);
	if (!ownerRigidBody)
	{
		return;
	}

	btRigidBody* ownerBulletRigidBody = ownerRigidBody->GetBulletRigidBody();
	const float ownerMass = ownerRigidBody->GetMass();

	if (ownerBulletRigidBody && ownerMass > 0.f && ownerBulletRigidBody->getInvMass() > 0.f)
	{
		btVector3 inertia;
		bulletCollisionShape_->calculateLocalInertia(ownerMass, inertia);
		ownerBulletRigidBody->setMassProps(ownerMass, inertia);
		ownerBulletRigidBody->updateInertiaTensor();
	}
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

	UpdateComponentToWorldTransformationMatrix();
}
	
void CollisionComponent::BeginGame()
{
	Component::BeginGame();
}

void CollisionComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
}

Component* CollisionComponent::Clone() const
{
	CollisionComponent* clonedComponent = new CollisionComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);
	clonedComponent->collisionGroup_ = collisionGroup_;
	clonedComponent->collisionMask_ = collisionMask_;

	return clonedComponent;
}
