#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__

#include "Core.h"

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
    PhysicsWorld(unsigned char maxContacts = PHYSICS_DEFAULT_MAX_CONTACTS, unsigned char iterations = PHYSICS_DEFAULT_ITEARATIONS);
    ~PhysicsWorld();

    void PhysicsTick(float deltaTime);

    void AddPhysicsObject(PhysicsObject* physicsObject);
    void RemovePhysicsObject(PhysicsObject* physicsObject);

    PhysicsObjectForceRegistry* GetPhysicsObjectForceRegistry() const
    {
        return physicsObjectForceRegistry_;
    }

protected:
    PhysicsObjectForceRegistry* physicsObjectForceRegistry_;

    typedef std::vector<PhysicsObject*> PhysicsObjects;
    PhysicsObjects physicsObjects_;

private:
    float remainingPhysicsTickDeltaTime_{ 0.f };
};

#endif