#include "pch.h"

#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Physics/ForceGenerators/PhysicsObjectForceGenerator.h"
#include "Goknar/Physics/PhysicsObject.h"

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