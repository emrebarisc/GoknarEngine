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

    constexpr int activeStateInActivation = ACTIVE_TAG;
    constexpr int activeStateInDisactivation = DISABLE_SIMULATION;

    constexpr int collisionFlagInActivation = !btCollisionObject::CF_NO_CONTACT_RESPONSE;
    constexpr int collisionFlagInDisactivation = btCollisionObject::CF_NO_CONTACT_RESPONSE;

    if (!GetIsInitialized())
    {
        if (isActive)
        {
            physicsObjectInitializationData_->activeState = activeStateInActivation;
            physicsObjectInitializationData_->collisionFlag = collisionFlagInActivation;
		}
		else
		{
			physicsObjectInitializationData_->activeState = activeStateInDisactivation;
            physicsObjectInitializationData_->collisionFlag = collisionFlagInDisactivation;
        }
	}
    else
    {
        if (isActive)
        {
            bulletCollisionObject_->setActivationState(activeStateInActivation);
            bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() & collisionFlagInActivation);
        }
        else
        {
            bulletCollisionObject_->setActivationState(activeStateInDisactivation);
            bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() | collisionFlagInDisactivation);
        }
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

    if (0 <= physicsObjectInitializationData_->activeState)
    {
        bulletCollisionObject_->setActivationState(physicsObjectInitializationData_->activeState);

        if (physicsObjectInitializationData_->activeState == ACTIVE_TAG)
        {
            bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() & physicsObjectInitializationData_->collisionFlag);
        }
        else if (physicsObjectInitializationData_->activeState == DISABLE_SIMULATION)
        {
            bulletCollisionObject_->setCollisionFlags(bulletCollisionObject_->getCollisionFlags() | physicsObjectInitializationData_->collisionFlag);
        }
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