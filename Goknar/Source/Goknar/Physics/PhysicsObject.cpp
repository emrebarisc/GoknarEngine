#include "pch.h"

#include "Goknar/Log.h"

#include "Goknar/Physics/PhysicsObject.h"

#define PHYSICS_TICKS_PER_SECOND 30
#define PHYSICS_TICK_DELTA_TIME 1.f / PHYSICS_TICKS_PER_SECOND

void PhysicsObject::Tick(float deltaTime)
{
    remainingPhysicsTickDeltaTime_ += deltaTime;
    while (PHYSICS_TICK_DELTA_TIME <= remainingPhysicsTickDeltaTime_)
    {
        PhysicsTick(PHYSICS_TICK_DELTA_TIME);

        remainingPhysicsTickDeltaTime_ -= PHYSICS_TICK_DELTA_TIME;
    }
}

void PhysicsObject::PhysicsTick(float deltaTime)
{
    if (inverseMass_ <= 0.0f)
    {
        return;
    }

    GOKNAR_CORE_ASSERT(0.f < deltaTime);

    // Update linear position.
    SetWorldPosition(worldPosition_ + velocity_ * deltaTime);

    // Work out the acceleration from the force
    Vector3 resultingAcc = acceleration_;
    resultingAcc += forceAccum_ * inverseMass_;

    // Update linear velocity from the acceleration.
    velocity_ += resultingAcc * deltaTime;

    // Impose drag.
    velocity_ *= std::powf(damping_, deltaTime);

    // Clear the forces.
    ClearAccumulator();
}

PhysicsObject::PhysicsObject() : ObjectBase()
{
    SetIsTickable(true);
}

void PhysicsObject::SetMass(const float mass)
{
    GOKNAR_CORE_ASSERT(mass != 0);
    inverseMass_ = 1.0f / mass;
}

float PhysicsObject::GetMass() const
{
    if (inverseMass_ == 0.f)
    {
        return FLT_MAX;
    }
    else
    {
        return (1.0f) / inverseMass_;
    }
}

void PhysicsObject::SetInverseMass(const float inverseMass)
{
    inverseMass_ = inverseMass;
}

void PhysicsObject::SetDamping(const float damping)
{
    damping_ = damping;
}

void PhysicsObject::SetVelocity(const Vector3 &velocity)
{
    velocity_ = velocity;
}

void PhysicsObject::SetVelocity(const float x, const float y, const float z)
{
    velocity_.x = x;
    velocity_.y = y;
    velocity_.z = z;
}

void PhysicsObject::SetAcceleration(const Vector3 &acceleration)
{
    acceleration_ = acceleration;
}

void PhysicsObject::SetAcceleration(const float x, const float y, const float z)
{
    acceleration_.x = x;
    acceleration_.y = y;
    acceleration_.z = z;
}

void PhysicsObject::ClearAccumulator()
{
    forceAccum_ = Vector3::ZeroVector;
}

void PhysicsObject::AddForce(const Vector3 &force)
{
    forceAccum_ += force;
}