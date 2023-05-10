#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__

#include "Core.h"

#include "Contacts/PhysicsContact.h"
#include "Contacts/ContactResolver.h"

class ContactGenerator;
class RigidBody;
class ForceRegistry;

class PhysicsObject;
class PhysicsObjectForceRegistry;

#define PHYSICS_TICKS_PER_SECOND 30
#define PHYSICS_TICK_DELTA_TIME 1.f / PHYSICS_TICKS_PER_SECOND
#define PHYSICS_DEFAULT_MAX_CONTACTS 8
#define PHYSICS_DEFAULT_ITEARATIONS 4

#define DEFAULT_GRAVITATIONAL_FORCE_MAGNITUDE 9.81f
#define DEFAULT_GRAVITATIONAL_FORCE Vector3::UpVector * -DEFAULT_GRAVITATIONAL_FORCE_MAGNITUDE

class GOKNAR_API PhysicsWorld
{   
public:
    PhysicsWorld(unsigned int maxContacts = PHYSICS_DEFAULT_MAX_CONTACTS, unsigned char iterations = PHYSICS_DEFAULT_ITEARATIONS);
    ~PhysicsWorld();

    void PhysicsTick(float deltaTime);

    void AddPhysicsObject(PhysicsObject* physicsObject);
    void RemovePhysicsObject(PhysicsObject* physicsObject);

    void AddRigidBody(RigidBody* rigidBody);
    void RemoveRigidBody(RigidBody* rigidBody);

    unsigned int GenerateContacts();

    PhysicsObjectForceRegistry* GetPhysicsObjectForceRegistry() const
    {
        return physicsObjectForceRegistry_;
    }

    ForceRegistry* GetForceRegistry() const
    {
        return forceRegistry_;
    }

protected:
    PhysicsObjectForceRegistry* physicsObjectForceRegistry_;
    ForceRegistry* forceRegistry_;

    typedef std::vector<PhysicsObject*> PhysicsObjects;
    PhysicsObjects physicsObjects_;

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

    ContactResolver contactResolver_;

    float remainingPhysicsTickDeltaTime_{ 0.f };

    unsigned int maxContacts_{ PHYSICS_DEFAULT_MAX_CONTACTS };

    bool calculateIterations_;
};

#endif