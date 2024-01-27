#include "pch.h"

#include "RigidBody.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "Bullet3Common/b3Vector3.h"

RigidBody::RigidBody() : ObjectBase()
{

}

RigidBody::~RigidBody()
{
}

void RigidBody::PreInit()
{
    ObjectBase::PreInit();
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
        bulletCollisionShape->calculateLocalInertia(mass_, localInertia_);
    }

    btTransform bulletTransform;
    bulletTransform.setIdentity();
    bulletTransform.setOrigin(btVector3{ worldPosition_.x, worldPosition_.y, worldPosition_.z });
    bulletTransform.setRotation(btQuaternion{ worldRotation_.x, worldRotation_.y, worldRotation_.z, worldRotation_.w });

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* bulletMotionState = new btDefaultMotionState(bulletTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(mass_, bulletMotionState, bulletCollisionShape, localInertia_);

    bulletRigidBody_ = new btRigidBody(rigidBodyInfo);

    engine->GetPhysicsWorld()->AddRigidBody(this);

    ObjectBase::Init();
}

void RigidBody::PostInit()
{
    ObjectBase::PostInit();

    if(0.01f < initialVelocity_.length2())
    {
        bulletRigidBody_->setLinearVelocity(initialVelocity_);
    }
    
    if(0.01f < initialAngularVelocity_.length2())
    {
        bulletRigidBody_->setAngularVelocity(initialAngularVelocity_);
    }    

    if(0.01f < initialCentralImpulse_.length2())
    {
        bulletRigidBody_->applyCentralImpulse(initialCentralImpulse_);
    }    

    if(0.01f < initialTorqueImpulse_.length2())
    {
        bulletRigidBody_->applyTorqueImpulse(initialTorqueImpulse_);
    }    

    if(0.01f < initialPushImpulse_.length2())
    {
        bulletRigidBody_->applyPushImpulse(initialPushImpulse_, initialPushImpulsePosition_);
    }    

    if(0.01f < initialForce_.length2())
    {
        bulletRigidBody_->applyForce(initialForce_, initialForcePosition_);
    }
}

void RigidBody::BeginGame()
{
    ObjectBase::BeginGame();
}

void RigidBody::Tick(float deltaTime)
{
    ObjectBase::Tick(deltaTime);
}

void RigidBody::PhysicsTick(float deltaTime)
{
    const btVector3& bulletWorldPosition = bulletRigidBody_->getCenterOfMassPosition();
    const btQuaternion& bulletWorldRotation = bulletRigidBody_->getOrientation();
    
    ObjectBase::SetWorldPosition(Vector3{bulletWorldPosition.x(), bulletWorldPosition.y(), bulletWorldPosition.z()}, false);
    ObjectBase::SetWorldRotation(Quaternion(bulletWorldRotation.x(), bulletWorldRotation.y(), bulletWorldRotation.z(), bulletWorldRotation.w()));
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

void RigidBody::SetWorldPosition(const Vector3& worldPosition, bool updateWorldTransformationMatrix)
{
    ObjectBase::SetWorldPosition(worldPosition, updateWorldTransformationMatrix);

    if (!GetIsInitialized())
    {
        return;
    }

    btTransform newBulletTransform = bulletRigidBody_->getCenterOfMassTransform();
    newBulletTransform.setOrigin(btVector3{ worldPosition.x, worldPosition.y, worldPosition.z });
    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);
}

void RigidBody::SetWorldRotation(const Quaternion& worldRotation, bool updateWorldTransformationMatrix)
{
    ObjectBase::SetWorldRotation(worldRotation, updateWorldTransformationMatrix);

    if (!GetIsInitialized())
    {
        return;
    }

    btTransform newBulletTransform = bulletRigidBody_->getCenterOfMassTransform();
    newBulletTransform.setRotation(btQuaternion{ worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w });
    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);
}

void RigidBody::SetLinearVelocity(const Vector3& velocity)
{
    initialVelocity_ = btVector3(velocity.x, velocity.y, velocity.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->setLinearVelocity(initialVelocity_);
}

void RigidBody::SetAngularVelocity(const Vector3& angularVelocity)
{
    initialAngularVelocity_ = btVector3(angularVelocity.x, angularVelocity.y, angularVelocity.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->setAngularVelocity(initialAngularVelocity_);
}

void RigidBody::ApplyForce(const Vector3& force, const Vector3& position)
{
    initialForce_ = btVector3(force.x, force.y, force.z);
    initialForcePosition_ = btVector3(position.x, position.y, position.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->applyForce(initialForce_, initialForcePosition_);
}

void RigidBody::ApplyCentralImpulse(const Vector3& impulse)
{
    initialCentralImpulse_ = btVector3(impulse.x, impulse.y, impulse.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->applyCentralImpulse(initialCentralImpulse_);
}

void RigidBody::ApplyTorqueImpulse(const Vector3& impulse)
{
    initialTorqueImpulse_ = btVector3(impulse.x, impulse.y, impulse.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->applyTorqueImpulse(initialTorqueImpulse_);
}

void RigidBody::ApplyPushImpulse(const Vector3& impulse, const Vector3& position)
{
    initialPushImpulse_ = btVector3(impulse.x, impulse.y, impulse.z);
    initialPushImpulsePosition_ = btVector3(position.x, position.y, position.z);

    if(!GetIsInitialized())
    {
        return;
    }

    bulletRigidBody_->applyPushImpulse(initialPushImpulse_, initialPushImpulsePosition_);
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
