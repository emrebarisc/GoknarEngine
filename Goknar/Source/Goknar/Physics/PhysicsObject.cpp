#include "pch.h"

#include "PhysicsObject.h"

#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "btBulletDynamicsCommon.h"

PhysicsObject::PhysicsObject()
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


void PhysicsObject::SetupPhysicsObjectInitializationData()
{
    bulletCollisionObject_->setCcdMotionThreshold(physicsObjectInitializationData_->ccdMotionThreshold);
    bulletCollisionObject_->setCcdSweptSphereRadius(physicsObjectInitializationData_->ccdSweptSphereRadius);

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

void PhysicsObject::SetLinearSleepingThreshold(float linearSleepingThreshold)
{
    if (GetIsInitialized())
    {
        return;
    }

    physicsObjectInitializationData_->linearSleepingThreshold = linearSleepingThreshold;
}

void PhysicsObject::SetAngularSleepingThreshold(float angularSleepingThreshold)
{
    if (GetIsInitialized())
    {
        return;
    }

    physicsObjectInitializationData_->angularSleepingThreshold = angularSleepingThreshold;
}