#include "pch.h"

#include "RigidBody.h"

#include "Engine.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "Bullet3Common/b3Vector3.h"

RigidBody::RigidBody() : ObjectBase()
{
    initializationData_ = new RigidBodyInitializationData();
}

RigidBody::~RigidBody()
{
    if(initializationData_)
    {
        delete initializationData_;
    }
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
        bulletCollisionShape->calculateLocalInertia(mass_, initializationData_->localInertia);
    }

    btTransform bulletTransform;
    bulletTransform.setIdentity();
    bulletTransform.setOrigin(btVector3{ worldPosition_.x, worldPosition_.y, worldPosition_.z });
    bulletTransform.setRotation(btQuaternion{ worldRotation_.x, worldRotation_.y, worldRotation_.z, worldRotation_.w });

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* bulletMotionState = new btDefaultMotionState(bulletTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo(mass_, bulletMotionState, bulletCollisionShape, initializationData_->localInertia);

    bulletRigidBody_ = new btRigidBody(rigidBodyInfo);

    engine->GetPhysicsWorld()->AddRigidBody(this);

    ObjectBase::Init();
}

void RigidBody::PostInit()
{
    ObjectBase::PostInit();

    if(0.01f < initializationData_->velocity.length2())
    {
        bulletRigidBody_->setLinearVelocity(initializationData_->velocity);
    }
    
    if(0.01f < initializationData_->angularVelocity.length2())
    {
        bulletRigidBody_->setAngularVelocity(initializationData_->angularVelocity);
    }    

    if(0.01f < initializationData_->centralImpulse.length2())
    {
        bulletRigidBody_->applyCentralImpulse(initializationData_->centralImpulse);
    }    

    if(0.01f < initializationData_->torqueImpulse.length2())
    {
        bulletRigidBody_->applyTorqueImpulse(initializationData_->torqueImpulse);
    }    

    if(0.01f < initializationData_->pushImpulse.length2())
    {
        bulletRigidBody_->applyPushImpulse(initializationData_->pushImpulse, initializationData_->pushImpulsePosition);
    }    

    if(0.01f < initializationData_->force.length2())
    {
        bulletRigidBody_->applyForce(initializationData_->force, initializationData_->forcePosition);
    }

    bulletRigidBody_->setLinearFactor(initializationData_->linearFactor);
    bulletRigidBody_->setAngularFactor(initializationData_->angularFactor);

    delete initializationData_;
    initializationData_ = nullptr;
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

void RigidBody::SetLinearFactor(const Vector3& linearFactor)
{
    btVector3 btLinearFactor = btVector3(linearFactor.x, linearFactor.y, linearFactor.z);

    if (!GetIsInitialized())
    {
        initializationData_->linearFactor = btLinearFactor;
        return;
    }

    bulletRigidBody_->setLinearFactor(btLinearFactor);
}

void RigidBody::SetAngularFactor(const Vector3& angularFactor)
{
    btVector3 btAngularFactor = btVector3(angularFactor.x, angularFactor.y, angularFactor.z);

    if (!GetIsInitialized())
    {
        initializationData_->angularFactor = btAngularFactor;
        return;
    }

    bulletRigidBody_->setAngularFactor(btAngularFactor);
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
    btVector3 btVelocity = btVector3(velocity.x, velocity.y, velocity.z);

    if(!GetIsInitialized())
    {
        initializationData_->velocity = btVelocity;
        return;
    }

    bulletRigidBody_->setLinearVelocity(btVelocity);
}

void RigidBody::SetAngularVelocity(const Vector3& angularVelocity)
{
    btVector3 btAngularVelocity = btVector3(angularVelocity.x, angularVelocity.y, angularVelocity.z);

    if(!GetIsInitialized())
    {
        initializationData_->angularVelocity = btAngularVelocity;
        return;
    }

    bulletRigidBody_->setAngularVelocity(btAngularVelocity);
}

void RigidBody::ApplyForce(const Vector3& force, const Vector3& position)
{
    btVector3 btForce = btVector3(force.x, force.y, force.z);
    btVector3 btForcePosition = btVector3(position.x, position.y, position.z);

    if(!GetIsInitialized())
    {
        initializationData_->force = btForce;
        initializationData_->forcePosition = btForcePosition;
        return;
    }

    bulletRigidBody_->applyForce(btForce, btForcePosition);
}

void RigidBody::ApplyCentralImpulse(const Vector3& impulse)
{
    btVector3 btImpulse = btVector3(impulse.x, impulse.y, impulse.z);

    if(!GetIsInitialized())
    {
        initializationData_->centralImpulse = btImpulse;
        return;
    }

    bulletRigidBody_->applyCentralImpulse(btImpulse);
}

void RigidBody::ApplyTorqueImpulse(const Vector3& impulse)
{
    btVector3 btImpulse = btVector3(impulse.x, impulse.y, impulse.z);

    if(!GetIsInitialized())
    {
        initializationData_->torqueImpulse = btImpulse;
        return;
    }

    bulletRigidBody_->applyTorqueImpulse(btImpulse);
}

void RigidBody::ApplyPushImpulse(const Vector3& impulse, const Vector3& position)
{
    btVector3 btPushImpulse = btVector3(impulse.x, impulse.y, impulse.z);
    btVector3 btPushImpulsePosition = btVector3(position.x, position.y, position.z);

    if(!GetIsInitialized())
    {
        initializationData_->pushImpulse = btPushImpulse;
        initializationData_->pushImpulsePosition = btPushImpulsePosition;
        return;
    }

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
