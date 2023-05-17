#include "pch.h"

#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Math/Matrix.h"
#include "Goknar/Physics/Contacts/ContactGenerator.h"
#include "Goknar/Physics/ForceGenerators/ForceGenerator.h"
#include "Goknar/Physics/ForceGenerators/PhysicsObjectForceGenerator.h"
#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/RigidBody.h"

PhysicsWorld::PhysicsWorld(unsigned int maxContacts, unsigned char iterations) :
    contactResolver_(iterations),
    firstBody_(nullptr),
    firstContactGen_(nullptr),
    maxContacts_(maxContacts)
{
    physicsObjectForceRegistry_ = new PhysicsObjectForceRegistry();
    forceRegistry_ = new ForceRegistry();

    contacts_ = new PhysicsContact[maxContacts];
    calculateIterations_ = (iterations == 0);
}

PhysicsWorld::~PhysicsWorld()
{
    delete[] contacts_;

    delete physicsObjectForceRegistry_;
    delete forceRegistry_;
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
    remainingPhysicsTickDeltaTime_ += deltaTime;
    while (PHYSICS_TICK_DELTA_TIME <= remainingPhysicsTickDeltaTime_)
    {
        forceRegistry_->UpdateForces(PHYSICS_TICK_DELTA_TIME);
        PhysicsObjects::iterator physicsObjectsIterator = physicsObjects_.begin();
        while (physicsObjectsIterator != physicsObjects_.end())
        {
            (*physicsObjectsIterator)->PhysicsTick(PHYSICS_TICK_DELTA_TIME);

            ++physicsObjectsIterator;
        }

        physicsObjectForceRegistry_->UpdateForces(PHYSICS_TICK_DELTA_TIME);
        RigidBodies::iterator rigidBodyIterator = rigidBodies_.begin();
        while (rigidBodyIterator != rigidBodies_.end())
        {
            (*rigidBodyIterator)->PhysicsTick(PHYSICS_TICK_DELTA_TIME);

            ++rigidBodyIterator;
        }


        // Generate contacts
        unsigned int usedContacts = GenerateContacts();

        // And process them
        if (calculateIterations_)
        {
            contactResolver_.SetIterations(usedContacts * 4);
        }
        contactResolver_.ResolveContacts(contacts_, usedContacts, deltaTime);

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

unsigned int PhysicsWorld::GenerateContacts()
{
    unsigned int limit = maxContacts_;
    PhysicsContact* nextContact = contacts_;

    ContactGenRegistration* reg = firstContactGen_;
    while (reg)
    {
        unsigned used = reg->gen->AddContact(nextContact, limit);
        limit -= used;
        nextContact += used;

        // We've run out of contacts to fill. This means we're missing
        // contacts.
        if (limit <= 0) break;

        reg = reg->next;
    }

    // Return the number of contacts used.
    return maxContacts_ - limit;
}