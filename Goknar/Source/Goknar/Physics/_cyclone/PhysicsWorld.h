#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__

#include "Core.h"

#include "PhysicsConstants.h"

#include "Contacts/PhysicsContact.h"
#include "Contacts/ContactResolver.h"

#include "Collisions/FineCollision.h"

class ContactGenerator;
class RigidBody;
class ForceRegistry;
class GOKNAR_API PhysicsWorld
{   
public:
    PhysicsWorld(unsigned int maxContacts = PHYSICS_DEFAULT_MAX_CONTACTS, unsigned int iterations = PHYSICS_DEFAULT_ITEARATIONS);
    ~PhysicsWorld();

    void PhysicsTick(float deltaTime);

    void AddRigidBody(RigidBody* rigidBody);
    void RemoveRigidBody(RigidBody* rigidBody);

    void AddCollision(CollisionPrimitive* collision);

    unsigned int GenerateContacts();

    ForceRegistry* GetForceRegistry() const
    {
        return forceRegistry_;
    }

protected:
    ForceRegistry* forceRegistry_;

    typedef std::vector<RigidBody*> RigidBodies;
    RigidBodies rigidBodies_;

private:
    struct BodyRegistration
    {
        RigidBody* body;
        BodyRegistration* next;
    };

    struct ContactGenRegistration
    {
        ContactGenerator* gen;
        ContactGenRegistration* next;
    };

    BodyRegistration* firstBody_;
    
    ContactGenRegistration* firstContactGen_;

    PhysicsContact* contacts_;

    CollisionData collisionData_;
    std::vector<CollisionPrimitive*> collisions_;

    ContactResolver contactResolver_;

    float remainingPhysicsTickDeltaTime_{ 0.f };

    unsigned int maxContacts_{ PHYSICS_DEFAULT_MAX_CONTACTS };

    bool calculateIterations_;
};

#endif