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
	bulletCollisionShape_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(worldScaling_));

	RigidBody* ownerRigidBody = reinterpret_cast<RigidBody*>(owner_);
	btRigidBody* ownerBulletRigidBody = ownerRigidBody->GetBulletRigidBody();

	if (ownerBulletRigidBody && ownerBulletRigidBody->getInvMass() > 0)
	{
		btVector3 inertia;
		bulletCollisionShape_->calculateLocalInertia(ownerBulletRigidBody->getMass(), inertia);
		ownerBulletRigidBody->setMassProps(ownerBulletRigidBody->getMass(), inertia);
		ownerBulletRigidBody->updateInertiaTensor();
	}
}

void CollisionComponent::PreInit()
{
	Component::PreInit();

	UpdateTransformation();
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

Component* CollisionComponent::Clone() const
{
	CollisionComponent* clonedComponent = new CollisionComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);
	clonedComponent->collisionGroup_ = collisionGroup_;
	clonedComponent->collisionMask_ = collisionMask_;

	return clonedComponent;
}
