#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__

#include "Core.h"

class PhysicsObject;

class GOKNAR_API PhysicsWorld
{   
public:
    PhysicsWorld(unsigned char maxContacts, unsigned char iterations = 0);

    void PhysicsTick(float deltaTime);

    void AddPhysicsObject(PhysicsObject* physicsObject);
    void RemovePhysicsObject(PhysicsObject* physicsObject);

protected:
    typedef std::vector<PhysicsObject*> PhysicsObjects;
    PhysicsObjects physicsObjects_;

private:
    float remainingPhysicsTickDeltaTime_{ 0.f };
};

#endif