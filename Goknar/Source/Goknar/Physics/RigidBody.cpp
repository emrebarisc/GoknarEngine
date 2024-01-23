#include "pch.h"

#include "RigidBody.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"

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

    CollisionComponent* collisionComponent = dynamic_cast<CollisionComponent*>(GetRootComponent());
    GOKNAR_ASSERT(collisionComponent != nullptr);

    btCollisionShape* bulletCollisionShape = collisionComponent->GetBulletCollisionShape();
    
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

void RigidBody::Init()
{
    ObjectBase::Init();
    
}

void RigidBody::PostInit()
{
    ObjectBase::PostInit();

}
