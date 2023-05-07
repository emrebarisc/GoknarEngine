#include "pch.h"

#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Physics/ForceGenerators/PhysicsObjectForceGenerator.h"
#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/RigidBody.h"

PhysicsWorld::PhysicsWorld(unsigned char maxContacts, unsigned char iterations)
{
    physicsObjectForceRegistry_ = new PhysicsObjectForceRegistry();
}

PhysicsWorld::~PhysicsWorld()
{
    delete physicsObjectForceRegistry_;
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
    remainingPhysicsTickDeltaTime_ += deltaTime;
    while (PHYSICS_TICK_DELTA_TIME <= remainingPhysicsTickDeltaTime_)
    {
        physicsObjectForceRegistry_->UpdateForces(PHYSICS_TICK_DELTA_TIME);

        PhysicsObjects::iterator physicsObjectsIterator = physicsObjects_.begin();
        while (physicsObjectsIterator != physicsObjects_.end())
        {
            (*physicsObjectsIterator)->PhysicsTick(PHYSICS_TICK_DELTA_TIME);

            ++physicsObjectsIterator;
        }

        RigidBodies::iterator rigidBodyIterator = rigidBodies_.begin();
        while (rigidBodyIterator != rigidBodies_.end())
        {
            (*rigidBodyIterator)->PhysicsTick(PHYSICS_TICK_DELTA_TIME);

            ++rigidBodyIterator;
        }

        remainingPhysicsTickDeltaTime_ -= PHYSICS_TICK_DELTA_TIME;
    }
}

void PhysicsWorld::AddPhysicsObject(PhysicsObject* physicsObject)
{
    physicsObjects_.push_back(physicsObject);
}

void PhysicsWorld::RemovePhysicsObject(PhysicsObject* physicsObject)
{
    PhysicsObjects::iterator physicsObjectsIterator = physicsObjects_.begin();
    while (physicsObjectsIterator != physicsObjects_.end())
    {
        if (*physicsObjectsIterator == physicsObject)
        {
            physicsObjects_.erase(physicsObjectsIterator);
            break;
        }

        ++physicsObjectsIterator;
    }
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
    rigidBodies_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
    RigidBodies::iterator rigidBodyIterator = rigidBodies_.begin();
    while (rigidBodyIterator != rigidBodies_.end())
    {
        if (*rigidBodyIterator == rigidBody)
        {
            rigidBodies_.erase(rigidBodyIterator);
            break;
        }

        ++rigidBodyIterator;
    }
}
