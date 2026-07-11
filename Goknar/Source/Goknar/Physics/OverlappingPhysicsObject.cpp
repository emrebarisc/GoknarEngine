#include "pch.h"

#include "OverlappingPhysicsObject.h"

#include "Engine.h"
#include "GoknarAssert.h"
#include "Components/CollisionComponent.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsUtils.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

OverlappingPhysicsObject::OverlappingPhysicsObject() : PhysicsObject()
{
}

OverlappingPhysicsObject::~OverlappingPhysicsObject()
{
}

ObjectBase* OverlappingPhysicsObject::Clone() const
{
    OverlappingPhysicsObject* clonedObject = new OverlappingPhysicsObject();
    CopyValuesTo(clonedObject);
    CopyPhysicsValuesTo(clonedObject);

    return clonedObject;
}

void OverlappingPhysicsObject::PreInit()
{
    PhysicsObject::PreInit();

    btCollisionShape* bulletCollisionShape = collisionComponent_->GetBulletCollisionShape();
    GOKNAR_ASSERT(bulletCollisionShape != nullptr);

    bulletCollisionObject_ = new btPairCachingGhostObject();
    bulletCollisionObject_->setCollisionShape(bulletCollisionShape);
    Vector3 actualWorldPosition;
    Vector3 actualWorldScaling;
    Quaternion actualWorldRotation;
    GetWorldTransformationMatrix().Decompose(actualWorldPosition, actualWorldScaling, actualWorldRotation);
    actualWorldRotation.Normalize();
    bulletCollisionObject_->setWorldTransform(
        btTransform(
            PhysicsUtils::FromQuaternionToBtQuaternion(actualWorldRotation),
            PhysicsUtils::FromVector3ToBtVector3(actualWorldPosition))
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
    PhysicsWorld* physicsWorld = engine->GetPhysicsWorld();
    if(physicsWorld)
    {
        physicsWorld->RemovePhysicsObject(this);
    }
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
        PhysicsUtils::FromQuaternionToBtQuaternion(collisionComponent_->GetWorldRotation()),
        PhysicsUtils::FromVector3ToBtVector3(collisionComponent_->GetWorldPosition()))
    );
}
