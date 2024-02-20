#include "pch.h"

#include "PhysicsObject.h"

#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "btBulletDynamicsCommon.h"

PhysicsObject::PhysicsObject() : ObjectBase()
{
    physicsObjectInitializationData_ = new PhysicsObjectInitializationData();
}

PhysicsObject::~PhysicsObject()
{
	delete physicsObjectInitializationData_;
}

void PhysicsObject::PreInit()
{
    ObjectBase::PreInit();

}

void PhysicsObject::Init()
{
    ObjectBase::Init();
}

void PhysicsObject::PostInit()
{
    ObjectBase::PostInit();

    SetupPhysicsObjectInitializationData();
}

void PhysicsObject::Destroy()
{
    ObjectBase::Destroy();
}

void PhysicsObject::SetIsActive(bool isActive)
{
    ObjectBase::SetIsActive(isActive);

    if(isActive)
    {
        bulletCollisionObject_->setActivationState(ACTIVE_TAG);
        bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() & !btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    else
    {
        bulletCollisionObject_->setActivationState(DISABLE_SIMULATION);
        bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

void PhysicsObject::SetupPhysicsObjectInitializationData()
{
    bulletCollisionObject_->setCcdMotionThreshold(physicsObjectInitializationData_->ccdMotionThreshold);
    bulletCollisionObject_->setCcdSweptSphereRadius(physicsObjectInitializationData_->ccdSweptSphereRadius);
    
    if(0 <= physicsObjectInitializationData_->collisionFlag)
    {
        bulletCollisionObject_->setCollisionFlags(physicsObjectInitializationData_->collisionFlag);
    }

    delete physicsObjectInitializationData_;
    physicsObjectInitializationData_ = nullptr;
}

void PhysicsObject::SetCcdMotionThreshold(float ccdMotionThreshold)
{
    if (!GetIsInitialized())
    {
        physicsObjectInitializationData_->ccdMotionThreshold = ccdMotionThreshold;
        return;
    }

    bulletCollisionObject_->setCcdMotionThreshold(ccdMotionThreshold);
}

void PhysicsObject::SetCcdSweptSphereRadius(float ccdSweptSphereRadius)
{
    if (!GetIsInitialized())
    {
        physicsObjectInitializationData_->ccdSweptSphereRadius = ccdSweptSphereRadius;
        return;
    }

    bulletCollisionObject_->setCcdSweptSphereRadius(ccdSweptSphereRadius);
}

void PhysicsObject::SetCollisionFlag(CollisionFlag collisionFlag)
{
    if (!GetIsInitialized())
    {
        physicsObjectInitializationData_->collisionFlag = (int)collisionFlag;
        return;
    }

    bulletCollisionObject_->setCollisionFlags((int)collisionFlag);
}

CollisionFlag PhysicsObject::GetCollisionFlag() const
{
    if (!GetIsInitialized())
    {
        return CollisionFlag::None;
    }

    return (CollisionFlag)bulletCollisionObject_->getCollisionFlags();
}