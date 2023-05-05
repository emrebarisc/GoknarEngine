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

    Vector3 velocity_{ Vector3::ZeroVector };
    Vector3 forceAccum_{ Vector3::ZeroVector };
    Vector3 acceleration_{ Vector3::ZeroVector };

    // Mass is in kg
    float inverseMass_{ 1.f };
    float damping_{ 0.99f };

    float remainingPhysicsTickDeltaTime_{ 0.f };
};

#endif