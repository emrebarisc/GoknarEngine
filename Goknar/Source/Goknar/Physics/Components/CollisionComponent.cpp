#include "pch.h"

#include "CollisionComponent.h"
#include "Engine.h"
#include "Physics/PhysicsObject.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"

float CollisionComponent::GetSafeBulletCollisionScale(float scale)
{
	if (GoknarMath::IsNanOrInf(scale))
	{
		return SMALLER_EPSILON;
	}

	if (GoknarMath::Abs(scale) < SMALLER_EPSILON)
	{
		return scale < 0.f ? -SMALLER_EPSILON : SMALLER_EPSILON;
	}

	return scale;
}

Vector3 CollisionComponent::GetSafeBulletCollisionScaling(const Vector3& scaling)
{
	return Vector3(
		GetSafeBulletCollisionScale(scaling.x),
		GetSafeBulletCollisionScale(scaling.y),
		GetSafeBulletCollisionScale(scaling.z)
	);
}

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

	const Vector3 safeWorldScaling = GetSafeBulletCollisionScaling(worldScaling_);
	bulletCollisionShape_->setLocalScaling(PhysicsUtils::FromVector3ToBtVector3(safeWorldScaling));

	PhysicsObject* ownerPhysicsObject = dynamic_cast<PhysicsObject*>(owner_);
	btCollisionObject* ownerBulletCollisionObject = ownerPhysicsObject ? ownerPhysicsObject->GetBulletCollisionObject() : nullptr;

	RigidBody* ownerRigidBody = dynamic_cast<RigidBody*>(owner_);
	if (ownerRigidBody)
	{
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

	if (ownerBulletCollisionObject)
	{
		ownerBulletCollisionObject->activate(true);

		PhysicsWorld* physicsWorld = engine ? engine->GetPhysicsWorld() : nullptr;
		if (physicsWorld && physicsWorld->GetBulletPhysicsWorld())
		{
			physicsWorld->GetBulletPhysicsWorld()->updateSingleAabb(ownerBulletCollisionObject);
		}
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
