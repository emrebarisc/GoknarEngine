#include "pch.h"

#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Physics/PhysicsObject.h"

PhysicsWorld::PhysicsWorld(unsigned char maxContacts, unsigned char iterations)
{
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
    remainingPhysicsTickDeltaTime_ += deltaTime;
    while (PHYSICS_TICK_DELTA_TIME <= remainingPhysicsTickDeltaTime_)
    {
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
