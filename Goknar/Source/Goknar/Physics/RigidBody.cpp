#include "pch.h"

#include "RigidBody.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "Bullet3Common/b3Vector3.h"

RigidBody::RigidBody()
{

}

RigidBody::~RigidBody()
{
    delete bulletRigidBody_;
}

void RigidBody::PreInit()
{
    ObjectBase::PreInit();
}

void RigidBody::Init()
{
    ObjectBase::Init();
    
}

void RigidBody::PostInit()
{
    ObjectBase::PostInit();

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
    bulletTransform.setOrigin(btVector3(worldPosition_.x, worldRotation_.y, worldRotation_.z));
    bulletTransform.setRotation(btQuaternion(worldRotation_.x, worldRotation_.y, worldRotation_.z, worldRotation_.w));

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* bulletMotionState = new btDefaultMotionState(bulletTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(mass_, bulletMotionState, bulletCollisionShape, localInertia_);
    bulletRigidBody_ = new btRigidBody(rigidBodyInfo);

    engine->GetPhysicsWorld()->AddRigidBody(this);
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
    
    ObjectBase::SetWorldPosition(Vector3{bulletWorldPosition.x(), bulletWorldPosition.y(), bulletWorldPosition.z()});
    ObjectBase::SetWorldRotation(Quaternion(bulletWorldRotation.x(), bulletWorldRotation.y(), bulletWorldRotation.z(), bulletWorldRotation.w()));
}

void RigidBody::SetWorldPosition(const Vector3& worldPosition, bool updateWorldTransformationMatrix)
{
    btTransform newBulletTransform(
        bulletRigidBody_->getOrientation(),
        btVector3(worldPosition.x, worldPosition.y, worldPosition.z)
    );

    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);

    ObjectBase::SetWorldPosition(worldPosition, updateWorldTransformationMatrix);
}

void RigidBody::SetWorldRotation(const Quaternion& worldRotation, bool updateWorldTransformationMatrix)
{
    btTransform newBulletTransform(
        btQuaternion(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w),
        bulletRigidBody_->getCenterOfMassPosition()
    );

    bulletRigidBody_->setCenterOfMassTransform(newBulletTransform);

    ObjectBase::SetWorldRotation(worldRotation, updateWorldTransformationMatrix);
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