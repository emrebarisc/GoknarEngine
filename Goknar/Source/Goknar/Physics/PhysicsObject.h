#ifndef __PHYSICSOBJECT_H__
#define __PHYSICSOBJECT_H__

#include "ObjectBase.h"

class GOKNAR_API PhysicsObject : public ObjectBase
{   
public:
    PhysicsObject();

    void SetMass(const float mass);
    float GetMass() const;

    void SetInverseMass(const float inverseMass);
    float GetInverseMass() const
    {
        return inverseMass_;
    }

    bool HasFiniteMass() const
    {
        return 0.0f <= inverseMass_;
    }

    void SetDamping(const float damping);
    float GetDamping() const
    {
        return damping_;
    }

    void SetVelocity(const Vector3 &velocity);
    void SetVelocity(const float x, const float y, const float z);
    void GetVelocity(Vector3 *velocity) const
    {
        *velocity = velocity_;
    }

    const Vector3& GetVelocity() const
    {
        return velocity_;
    }

    void SetAcceleration(const Vector3 &acceleration);
    void SetAcceleration(const float x, const float y, const float z);
    void GetAcceleration(Vector3 *acceleration) const
    {
        *acceleration = acceleration_;
    }

    const Vector3& GetAcceleration() const
    {
        return acceleration_;
    }

    void ClearAccumulator();
    void AddForce(const Vector3 &force);

protected:
	virtual void Tick(float deltaTime) override;
    void PhysicsTick(float deltaTime);

    // Mass is in kg
    float inverseMass_;
    float damping_;
    Vector3 velocity_;
    Vector3 forceAccum_;
    Vector3 acceleration_;
};

#endif