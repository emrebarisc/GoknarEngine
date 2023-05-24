#include "pch.h"

#include "Goknar/Physics/RigidBody.h"

#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Physics/PhysicsWorld.h"

/**
 * Internal function that checks the validity of an inverse inertia tensor.
 */
static inline void _checkInverseInertiaTensor(const Matrix3x3& iitWorld)
{
    // TODO: Perform a validity check in an assert.
}

/**
 * Internal function to do an intertia tensor transform by a quaternion.
 * Note that the implementation of this function was created by an
 * automated code-generator and optimizer.
 */
static inline void _transformInertiaTensor(
    Matrix3x3& iitWorld,
    const Quaternion& q,
    const Matrix3x3& iitBody,
    const Matrix& rotmat)
{
    float t4 = rotmat[0] * iitBody[0] +
        rotmat[1] * iitBody[3] +
        rotmat[2] * iitBody[6];
    float t9 = rotmat[0] * iitBody[1] +
        rotmat[1] * iitBody[4] +
        rotmat[2] * iitBody[7];
    float t14 = rotmat[0] * iitBody[2] +
        rotmat[1] * iitBody[5] +
        rotmat[2] * iitBody[8];
    float t28 = rotmat[4] * iitBody[0] +
        rotmat[5] * iitBody[3] +
        rotmat[6] * iitBody[6];
    float t33 = rotmat[4] * iitBody[1] +
        rotmat[5] * iitBody[4] +
        rotmat[6] * iitBody[7];
    float t38 = rotmat[4] * iitBody[2] +
        rotmat[5] * iitBody[5] +
        rotmat[6] * iitBody[8];
    float t52 = rotmat[8] * iitBody[0] +
        rotmat[9] * iitBody[3] +
        rotmat[10] * iitBody[6];
    float t57 = rotmat[8] * iitBody[1] +
        rotmat[9] * iitBody[4] +
        rotmat[10] * iitBody[7];
    float t62 = rotmat[8] * iitBody[2] +
        rotmat[9] * iitBody[5] +
        rotmat[10] * iitBody[8];

    iitWorld.m[0] = t4 * rotmat[0] +
        t9 * rotmat[1] +
        t14 * rotmat[2];
    iitWorld.m[1] = t4 * rotmat[4] +
        t9 * rotmat[5] +
        t14 * rotmat[6];
    iitWorld.m[2] = t4 * rotmat[8] +
        t9 * rotmat[9] +
        t14 * rotmat[10];
    iitWorld.m[3] = t28 * rotmat[0] +
        t33 * rotmat[1] +
        t38 * rotmat[2];
    iitWorld.m[4] = t28 * rotmat[4] +
        t33 * rotmat[5] +
        t38 * rotmat[6];
    iitWorld.m[5] = t28 * rotmat[8] +
        t33 * rotmat[9] +
        t38 * rotmat[10];
    iitWorld.m[6] = t52 * rotmat[0] +
        t57 * rotmat[1] +
        t62 * rotmat[2];
    iitWorld.m[7] = t52 * rotmat[4] +
        t57 * rotmat[5] +
        t62 * rotmat[6];
    iitWorld.m[8] = t52 * rotmat[8] +
        t57 * rotmat[9] +
        t62 * rotmat[10];
}

RigidBody::RigidBody()
{
}

void RigidBody::Init()
{
    if (isPhysicsEnabled_)
    {
        engine->GetPhysicsWorld()->AddRigidBody(this);
    }

    if (isKinematic_)
    {
        SetBlockInertiaTensor(GetWorldScaling() * 0.5f, GetMass());
    }
    else
    {
        float coeff = GetMass();
        SetInertiaTensorCoeffs(coeff, coeff, coeff);
    }
}

void RigidBody::SetIsAwake(const bool isAwake)
{
    if (isAwake)
    {
        isAwake_ = true;

        // Add a bit of motion to avoid it falling asleep immediately.
        motion_ = RIGID_BODY_SLEEP_EPSILON * 2.0f;
    }
    else
    {
        isAwake_ = false;
        velocity_ = Vector3::ZeroVector;
    }
}

void RigidBody::SetIsPhysicsEnabled(bool isPhysicsEnabled)
{
    if (isPhysicsEnabled_ != isPhysicsEnabled)
    {
        if (isPhysicsEnabled)
        {
            engine->GetPhysicsWorld()->AddRigidBody(this);
        }
        else
        {
            engine->GetPhysicsWorld()->RemoveRigidBody(this);
        }

        isPhysicsEnabled_ = isPhysicsEnabled;
    }
}

void RigidBody::PhysicsTick(float deltaTime)
{
    if (!isAwake_) return;

    if (isGravityEnabled_ && HasFiniteMass())
    {
        AddForce(DEFAULT_GRAVITATIONAL_FORCE * GetMass());
    }

    // Calculate linear acceleration from force inputs.
    lastFrameAcceleration_ = acceleration_;
    lastFrameAcceleration_ += forceAccum_ * inverseMass_;

    // Calculate angular acceleration from torque inputs.
    Vector3 angularAcceleration = inverseInertiaTensorWorld_ * torqueAccum_;

    // Adjust velocities
    // Update linear velocity from both acceleration and impulse.
    velocity_ += lastFrameAcceleration_ * deltaTime;

    // Update angular velocity from both acceleration and impulse.
    eulerRotation_ += angularAcceleration * deltaTime;

    // Impose drag.
    velocity_ *= std::powf(linearDamping_, deltaTime);
    eulerRotation_ *= std::powf(angularDamping_, deltaTime);

    // Adjust positions
    // Update linear position.
    worldPosition_ += velocity_ * deltaTime;

    // Update angular position.
    worldRotation_ += Quaternion::FromEulerDegrees(eulerRotation_) * deltaTime;

    // Normalise the orientation, and update the matrices with the new
    // position and orientation
    CalculateDerivedData();

    // Clear accumulators.
    ClearAccumulators();

    // Update the kinetic energy store, and possibly put the body to
    // sleep.
    if (canSleep_)
    {
        float currentMotion = velocity_.Dot(velocity_) + eulerRotation_.Dot(eulerRotation_);

        float bias = std::powf(0.5f, deltaTime);
        motion_ = bias * motion_ + (1.f - bias) * currentMotion;

        if (motion_ < RIGID_BODY_SLEEP_EPSILON)
        {
            SetIsAwake(false);
        }
        else if (10.f * RIGID_BODY_SLEEP_EPSILON < motion_)
        {
            motion_ = 10 * RIGID_BODY_SLEEP_EPSILON;
        }
    }
}

void RigidBody::CalculateDerivedData()
{
    worldRotation_.Normalize();

    worldTransformationMatrix_ = Matrix::GetPositionMatrix(worldPosition_) * worldRotation_.GetMatrix();
    UpdateChildrenTransformations();

    _transformInertiaTensor(
        inverseInertiaTensorWorld_,
        worldRotation_,
        inverseInertiaTensor_,
        worldTransformationMatrix_);
}

void RigidBody::SetMass(const float mass)
{
    GOKNAR_ASSERT(mass != 0);
    inverseMass_ = 1.f / mass;
}

float RigidBody::GetMass() const
{
    if (inverseMass_ == 0)
    {
        return FLT_MAX;
    }
    else
    {
        return 1.f / inverseMass_;
    }
}

void RigidBody::SetInertiaTensor(const Matrix3x3& inertiaTensor)
{
    inertiaTensor.GetInverse(inverseInertiaTensor_);
    _checkInverseInertiaTensor(inverseInertiaTensor_);
}

void RigidBody::SetInverseInertiaTensor(const Matrix3x3& inverseInertiaTensor)
{
    _checkInverseInertiaTensor(inverseInertiaTensor);
    inverseInertiaTensor_ = inverseInertiaTensor;
}

void RigidBody::ClearAccumulators()
{
    forceAccum_ = Vector3::ZeroVector;
    torqueAccum_ = Vector3::ZeroVector;
}

void RigidBody::AddForce(const Vector3& force)
{
    forceAccum_ += force;
    SetIsAwake(true);
}

void RigidBody::AddForceAtBodyPoint(const Vector3& force, const Vector3& point)
{
    // Convert to coordinates relative to center of mass.
    Vector3 pt = worldTransformationMatrix_ * Vector4(point, 1.f);
    AddForceAtPoint(force, pt);
}

void RigidBody::AddForceAtPoint(const Vector3& force, const Vector3& point)
{
    // Convert to coordinates relative to center of mass.
    Vector3 pt = point;
    pt -= worldPosition_;

    forceAccum_ += force;
    torqueAccum_ += pt.Cross(force);

    isAwake_ = true;
}

void RigidBody::AddTorque(const Vector3& torque)
{
    torqueAccum_ += torque;
    isAwake_ = true;
}
