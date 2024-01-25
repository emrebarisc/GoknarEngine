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

void RigidBody::ApplyForce(const Vector3& force, const Vector3& position)
{
    bulletRigidBody_->applyForce(
        btVector3(force.x, force.y, force.z),
        btVector3(position.x, position.y, position.z)
        );
}

void RigidBody::ApplyCentralImpulse(const Vector3& impulse)
{
    bulletRigidBody_->applyCentralImpulse(
        btVector3(impulse.x, impulse.y, impulse.z)
        );
}

void RigidBody::ApplyTorqueImpulse(const Vector3& impulse)
{
    bulletRigidBody_->applyTorqueImpulse(
        btVector3(impulse.x, impulse.y, impulse.z)
        );
}

void RigidBody::ApplyPushImpulse(const Vector3& impulse, const Vector3& position)
{
    bulletRigidBody_->applyPushImpulse(
        btVector3(impulse.x, impulse.y, impulse.z), 
        btVector3(position.x, position.y, position.z)
        );
}

void RigidBody::ClearForces()
{
    bulletRigidBody_->clearForces();
}

void RigidBody::ClearGravity()
{
    bulletRigidBody_->clearGravity();
}
