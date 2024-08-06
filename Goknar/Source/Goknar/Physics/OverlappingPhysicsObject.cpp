#include "pch.h"

#include "OverlappingPhysicsObject.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsUtils.h"

#include "btBulletDynamicsCommon.h"
#include "Bullet3Common/b3Vector3.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

OverlappingPhysicsObject::OverlappingPhysicsObject() : PhysicsObject()
{
}

OverlappingPhysicsObject::~OverlappingPhysicsObject()
{
}

void OverlappingPhysicsObject::PreInit()
{
    PhysicsObject::PreInit();

    btCollisionShape* bulletCollisionShape = collisionComponent_->GetBulletCollisionShape();
    GOKNAR_ASSERT(bulletCollisionShape != nullptr);

    bulletCollisionObject_ = new btPairCachingGhostObject();
    bulletCollisionObject_->setCollisionShape(bulletCollisionShape);
    bulletCollisionObject_->setWorldTransform(
        btTransform(
            PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation_),
            PhysicsUtils::FromVector3ToBtVector3(worldPosition_))
    );

    engine->GetPhysicsWorld()->AddPhysicsObject(this);

}

void OverlappingPhysicsObject::Init()
{
    PhysicsObject::Init();
}

void OverlappingPhysicsObject::PostInit()
{
    PhysicsObject::PostInit();
}

void OverlappingPhysicsObject::Destroy()
{
    PhysicsObject::Destroy();
}

void OverlappingPhysicsObject::DestroyInner()
{
    engine->GetPhysicsWorld()->RemovePhysicsObject(this);
    PhysicsObject::DestroyInner();
}

void OverlappingPhysicsObject::BeginGame()
{
    PhysicsObject::BeginGame();
}

void OverlappingPhysicsObject::Tick(float deltaTime)
{
    PhysicsObject::Tick(deltaTime);
}

void OverlappingPhysicsObject::PhysicsTick(float deltaTime)
{
	PhysicsObject::PhysicsTick(deltaTime);
}

void OverlappingPhysicsObject::UpdateWorldTransformationMatrix()
{
    PhysicsObject::UpdateWorldTransformationMatrix();

    if(!GetIsInitialized())
	{
		return;
	}

    btTransform collisionObjectTransform;
    bulletCollisionObject_->setWorldTransform(btTransform(
        PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation_),
        PhysicsUtils::FromVector3ToBtVector3(worldPosition_))
    );
}