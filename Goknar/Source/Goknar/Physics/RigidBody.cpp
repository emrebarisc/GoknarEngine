#include "pch.h"

#include "RigidBody.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsUtils.h"

#include "btBulletDynamicsCommon.h"
#include "Bullet3Common/b3Vector3.h"

RigidBody::RigidBody() : PhysicsObject()
{
    rigidBodyInitializationData_ = new RigidBodyInitializationData();
}

RigidBody::~RigidBody()
{
    delete rigidBodyInitializationData_;
    delete bulletMotionState_;
}

void RigidBody::PreInit()
{
    PhysicsObject::PreInit();
}

void RigidBody::Init()
{
    CollisionComponent* collisionComponent = dynamic_cast<CollisionComponent*>(GetRootComponent());
    GOKNAR_ASSERT(collisionComponent != nullptr);

    btCollisionShape* bulletCollisionShape = collisionComponent->GetBulletCollisionShape();
    GOKNAR_ASSERT(bulletCollisionShape != nullptr);

    bool isDynamic = (mass_ != 0.f);
    if (isDynamic)
    {
        bulletCollisionShape->calculateLocalInertia(mass_, rigidBodyInitializationData_->localInertia);
    }

    btTransform bulletTransform;
    bulletTransform.setIdentity();
    bulletTransform.setOrigin(PhysicsUtils::FromVector3ToBtVector3(worldPosition_));
    bulletTransform.setRotation(PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation_));

    bulletMotionState_ = new btDefaultMotionState(bulletTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(mass_, bulletMotionState_, bulletCollisionShape, rigidBodyInitializationData_->localInertia);

    if (0.f <= rigidBodyInitializationData_->linearSleepingThreshold)
    {
        rigidBodyInfo.m_linearSleepingThreshold = rigidBodyInitializationData_->linearSleepingThreshold;
    }

    if (0.f <= rigidBodyInitializationData_->angularSleepingThreshold)
    {
        rigidBodyInfo.m_angularSleepingThreshold = rigidBodyInitializationData_->angularSleepingThreshold;
    }

    bulletRigidBody_ = new btRigidBody(rigidBodyInfo);
    bulletCollisionObject_ = bulletRigidBody_;

    engine->GetPhysicsWorld()->AddRigidBody(this);

    PhysicsObject::Init();
}

void RigidBody::PostInit()
{
    PhysicsObject::PostInit();

    SetupRigidBodyInitializationData();
}

void RigidBody::Destroy()
{
    PhysicsObject::Destroy();
}

void RigidBody::DestroyInner()
{
    engine->GetPhysicsWorld()->RemoveRigidBody(this);
    PhysicsObject::DestroyInner();
}

void RigidBody::BeginGame()
{
    PhysicsObject::BeginGame();
}

void RigidBody::Tick(float deltaTime)
{
    PhysicsObject::Tick(deltaTime);
}

void RigidBody::PhysicsTick(float deltaTime)
{
    const btVector3& bulletWorldPosition = bulletRigidBody_->getCenterOfMassPosition();
    const btQuaternion& bulletWorldRotation = bulletRigidBody_->getOrientation();

    PhysicsObject::SetWorldPosition(PhysicsUtils::FromBtVector3ToVector3(bulletWorldPosition), false);
    PhysicsObject::SetWorldRotation(PhysicsUtils::FromBtQuaternionToQuaternion(bulletWorldRotation));
}

void RigidBody::SetupRigidBodyInitializationData()
{
    if (0.01f < rigidBodyInitializationData_->velocity.length2())
    {
        bulletRigidBody_->setLinearVelocity(rigidBodyInitializationData_->velocity);
    }

    if (0.01f < rigidBodyInitializationData_->angularVelocity.length2())
    {
        bulletRigidBody_->setAngularVelocity(rigidBodyInitializationData_->angularVelocity);
    }

    if (0.01f < rigidBodyInitializationData_->centralImpulse.length2())
    {
        bulletRigidBody_->applyCentralImpulse(rigidBodyInitializationData_->centralImpulse);
    }

    if (0.01f < rigidBodyInitializationData_->torqueImpulse.length2())
    {
        bulletRigidBody_->applyTorqueImpulse(rigidBodyInitializationData_->torqueImpulse);
    }

    if (0.01f < rigidBodyInitializationData_->pushImpulse.length2())
    {
        bulletRigidBody_->applyPushImpulse(rigidBodyInitializationData_->pushImpulse, rigidBodyInitializationData_->pushImpulsePosition);
    }

    if (0.01f < rigidBodyInitializationData_->force.length2())
    {
        bulletRigidBody_->applyForce(rigidBodyInitializationData_->force, rigidBodyInitializationData_->forcePosition);
    }

    if (0.01f < rigidBodyInitializationData_->torque.length2())
    {
        bulletRigidBody_->applyTorque(rigidBodyInitializationData_->torque);
    }

    bulletRigidBody_->setLinearFactor(rigidBodyInitializationData_->linearFactor);
    bulletRigidBody_->setAngularFactor(rigidBodyInitializationData_->angularFactor);

    delete rigidBodyInitializationData_;
    rigidBodyInitializationData_ = nullptr;
}

void RigidBody::SetMass(float mass)
{
    mass_ = mass;

    if (!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->setMassProps(mass_, bulletRigidBody_->getLocalInertia());
}

void RigidBody::SetLinearFactor(const Vector3& linearFactor)
{
    btVector3 btLinearFactor = PhysicsUtils::FromVector3ToBtVector3(linearFactor);

    if (!GetIsInitialized())
    {
        rigidBodyInitializationData_->linearFactor = btLinearFactor;
        return;
    }

    bulletRigidBody_->setLinearFactor(btLinearFactor);
}

void RigidBody::SetAngularFactor(const Vector3& angularFactor)
{
    btVector3 btAngularFactor = PhysicsUtils::FromVector3ToBtVector3(angularFactor);

    if (!GetIsInitialized())
    {
        rigidBodyInitializationData_->angularFactor = btAngularFactor;
        return;
    }

    bulletRigidBody_->setAngularFactor(btAngularFactor);
}

void RigidBody::SetWorldPosition(const Vector3& worldPosition, bool updateWorldTransformationMatrix)
{
    PhysicsObject::SetWorldPosition(worldPosition, updateWorldTransformationMatrix);

    if (!GetIsInitialized())
    {
        return;
    }

    btTransform newBulletTransform = bulletRigidBody_->getCenterOfMassTransform();
    newBulletTransform.setOrigin(PhysicsUtils::FromVector3ToBtVector3(worldPosition));
    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);
}

void RigidBody::SetWorldRotation(const Quaternion& worldRotation, bool updateWorldTransformationMatrix)
{
    PhysicsObject::SetWorldRotation(worldRotation, updateWorldTransformationMatrix);

    if (!GetIsInitialized())
    {
        return;
    }

    btTransform newBulletTransform = bulletRigidBody_->getCenterOfMassTransform();
    newBulletTransform.setRotation(PhysicsUtils::FromQuaternionToBtQuaternion(worldRotation));
    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);
}

void RigidBody::SetIsActive(bool isActive)
{
    PhysicsObject::SetIsActive(isActive);

    //bulletRigidBody_->setActivationState(isActive ? : );
}

void RigidBody::SetLinearVelocity(const Vector3& velocity)
{
    btVector3 btVelocity = PhysicsUtils::FromVector3ToBtVector3(velocity);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->velocity = btVelocity;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->setLinearVelocity(btVelocity);
}

void RigidBody::SetAngularVelocity(const Vector3& angularVelocity)
{
    btVector3 btAngularVelocity = PhysicsUtils::FromVector3ToBtVector3(angularVelocity);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->angularVelocity = btAngularVelocity;
        return;
    }

    bulletRigidBody_->setAngularVelocity(btAngularVelocity);
}

void RigidBody::ApplyForce(const Vector3& force, const Vector3& position)
{
    btVector3 btForce = PhysicsUtils::FromVector3ToBtVector3(force);
    btVector3 btForcePosition = PhysicsUtils::FromVector3ToBtVector3(position);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->force = btForce;
        rigidBodyInitializationData_->forcePosition = btForcePosition;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->applyForce(btForce, btForcePosition);
}

void RigidBody::ApplyTorque(const Vector3& torque)
{
    btVector3 btTorque = PhysicsUtils::FromVector3ToBtVector3(torque);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->torque = btTorque;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->applyTorque(btTorque);
}

void RigidBody::ApplyCentralImpulse(const Vector3& impulse)
{
    btVector3 btImpulse = PhysicsUtils::FromVector3ToBtVector3(impulse);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->centralImpulse = btImpulse;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->applyCentralImpulse(btImpulse);
}

void RigidBody::ApplyTorqueImpulse(const Vector3& impulse)
{
    btVector3 btImpulse = PhysicsUtils::FromVector3ToBtVector3(impulse);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->torqueImpulse = btImpulse;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->applyTorqueImpulse(btImpulse);
}

void RigidBody::ApplyPushImpulse(const Vector3& impulse, const Vector3& position)
{
    btVector3 btPushImpulse = PhysicsUtils::FromVector3ToBtVector3(impulse);
    btVector3 btPushImpulsePosition = PhysicsUtils::FromVector3ToBtVector3(position);

    if(!GetIsInitialized())
    {
        rigidBodyInitializationData_->pushImpulse = btPushImpulse;
        rigidBodyInitializationData_->pushImpulsePosition = btPushImpulsePosition;
        return;
    }

    bulletRigidBody_->activate();
    bulletRigidBody_->applyPushImpulse(btPushImpulse, btPushImpulsePosition);
}

void RigidBody::ClearForces()
{
    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->clearForces();
}

void RigidBody::ClearGravity()
{
    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->clearGravity();
}

void RigidBody::SetLinearSleepingThreshold(float linearSleepingThreshold)
{
    if (GetIsInitialized())
    {
        return;
    }

    rigidBodyInitializationData_->linearSleepingThreshold = linearSleepingThreshold;
}

void RigidBody::SetAngularSleepingThreshold(float angularSleepingThreshold)
{
    if (GetIsInitialized())
    {
        return;
    }

    rigidBodyInitializationData_->angularSleepingThreshold = angularSleepingThreshold;
}
