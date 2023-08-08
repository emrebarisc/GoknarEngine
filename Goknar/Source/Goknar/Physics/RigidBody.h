#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

#include "Core.h"

#include "Goknar/ObjectBase.h"

#define RIGID_BODY_SLEEP_EPSILON 0.3f

class GOKNAR_API RigidBody : public ObjectBase
{   
    friend class PhysicsWorld;

public:
    RigidBody();

    virtual void Init() override;

    virtual void SetWorldRotation(const Quaternion& rotation, bool updateWorldTransformationMatrix = true) override;

    void SetMass(const float mass);
    float GetMass() const;

    void SetInverseMass(const float inverseMass)
    {
        inverseMass_ = inverseMass;
    }

    float GetInverseMass() const
    {
        return inverseMass_;
    }

    bool HasFiniteMass() const
    {
        return 0.f <= inverseMass_;
    }

    void SetDamping(const float linearDamping, const float angularDamping)
    {
        linearDamping_ = linearDamping;
        angularDamping_ = angularDamping;
    }

    void SetLinearDamping(const float linearDamping)
    {
        linearDamping_ = linearDamping;
    }

    float GetLinearDamping() const
    {
        return linearDamping_;
    }

    void SetAngularDamping(const float angularDamping)
    {
        angularDamping_ = angularDamping;
    }

    float GetAngularDamping() const
    {
        return angularDamping_;
    }

    void SetVelocity(const Vector3& velocity)
    {
        velocity_ = velocity;
    }

    void SetVelocity(const float x, const float y, const float z)
    {
        velocity_ = Vector3{x, y, z};
    }
    
    void GetVelocity(Vector3* velocity) const
    {
        *velocity = velocity_;
    }

    const Vector3& GetVelocity() const
    {
        return velocity_;
    }

    void SetAcceleration(const Vector3& acceleration)
    {
        acceleration_ = acceleration;
    }

    void SetAcceleration(const float x, const float y, const float z)
    {
        acceleration_ = Vector3{x, y, z};
    }

    void GetAcceleration(Vector3* acceleration) const
    {
        *acceleration = acceleration_;
    }

    const Vector3& GetAcceleration() const
    {
        return acceleration_;
    }

    bool GetIsGravityEnabled() const
    {
        return isGravityEnabled_;
    }

    void SetIsGravityEnabled(bool isGravityEnabled)
    {
        isGravityEnabled_ = isGravityEnabled;
    }

    void SetInertiaTensorCoeffs(float ix, float iy, float iz, float ixy = 0.f, float ixz = 0.f, float iyz = 0.f)
    {
        Matrix3x3 inertiaTensor;

        inertiaTensor.m[0] = ix;
        inertiaTensor.m[1] = inertiaTensor.m[3] = -ixy;
        inertiaTensor.m[2] = inertiaTensor.m[6] = -ixz;
        inertiaTensor.m[4] = iy;
        inertiaTensor.m[5] = inertiaTensor.m[7] = -iyz;
        inertiaTensor.m[8] = iz;

        SetInertiaTensor(inertiaTensor);
    }

    void SetBlockInertiaTensor(const Vector3& halfSizes, float mass)
    {
        Vector3 squares = halfSizes * halfSizes;
        SetInertiaTensorCoeffs( 0.3f * mass * (squares.y + squares.z),
                                0.3f * mass * (squares.x + squares.z),
                                0.3f * mass * (squares.x + squares.y));
    }

    /**
     * Sets the intertia tensor for the rigid body.
     *
     * @param inertiaTensor The inertia tensor for the rigid
     * body. This must be a full rank matrix and must be
     * invertible.
     *
     * @warning This invalidates internal data for the rigid body.
     * Either an integration function, or the calculateInternals
     * function should be called before trying to get any settings
     * from the rigid body.
     */
    void SetInertiaTensor(const Matrix3x3& inertiaTensor);

    void GetInertiaTensor(Matrix3x3* inertiaTensor) const
    {
        inverseInertiaTensor_.GetInverse(*inertiaTensor);
    }

    Matrix3x3 GetInertiaTensor() const
    {
        return inverseInertiaTensor_.GetInverse();
    }

    void GetInertiaTensorWorld(Matrix3x3* inertiaTensor) const
    {
        inverseInertiaTensorWorld_.GetInverse(*inertiaTensor);
    }
    
    Matrix3x3 GetInertiaTensorWorld() const
    {
        return inverseInertiaTensorWorld_.GetInverse();
    }

    /**
     * Sets the inverse intertia tensor for the rigid body.
     *
     * @param inverseInertiaTensor The inverse inertia tensor for
     * the rigid body. This must be a full rank matrix and must be
     * invertible.
     *
     * @warning This invalidates internal data for the rigid body.
     * Either an integration function, or the calculateInternals
     * function should be called before trying to get any settings
     * from the rigid body.
     */
    void SetInverseInertiaTensor(const Matrix3x3& inverseInertiaTensor);

    void GetInverseInertiaTensor(Matrix3x3* inverseInertiaTensor) const
    {
        *inverseInertiaTensor = inverseInertiaTensor_;
    }

    const Matrix3x3& GetInverseInertiaTensor() const
    {
        return inverseInertiaTensor_;
    }

    void GetInverseInertiaTensorWorld(Matrix3x3* inverseInertiaTensor) const
    {
        *inverseInertiaTensor = inverseInertiaTensorWorld_;
    }

    const Matrix3x3& GetInverseInertiaTensorWorld() const
    {
        return inverseInertiaTensorWorld_;
    }

    void GetLastFrameAcceleration(Vector3* acceleration) const
    {
        *acceleration = lastFrameAcceleration_;
    }

    const Vector3& GetLastFrameAcceleration() const
    {
        return lastFrameAcceleration_;
    }

    bool GetIsAwake() const
    {
        return isAwake_;
    }

    void SetIsAwake(const bool isAwake = true);

    bool GetCanSleep() const
    {
        return canSleep_;
    }

    void SetCanSleep(const bool canSleep = true)
    {
        canSleep_ = canSleep;
    }

    bool GetIsPhysicsEnabled() const
    {
        return isPhysicsEnabled_;
    }

    void SetIsPhysicsEnabled(bool isPhysicsEnabled);

    void SetIsKinematic(bool isKinematic)
    {
        isKinematic_ = isKinematic;
    }

    bool GetIsKinematic() const
    {
        return isKinematic_;
    }

    void ClearAccumulators();

    void AddForce(const Vector3& force);
    void AddForceAtPoint(const Vector3& force, const Vector3& point);
    void AddForceAtBodyPoint(const Vector3& force, const Vector3& point);

    void AddTorque(const Vector3& torque);

    void AddVelocity(const Vector3& velocityAddition)
    {
        velocity_ += velocityAddition;
    }

    /**
     * Calculates internal data from state data. This should be called
     * after the body's state is altered directly (it is called
     * automatically during integration). If you change the body's state
     * and then intend to integrate before querying any data (such as
     * the transform matrix), then you can ommit this step.
     */
    void CalculateDerivedData();

protected:
    virtual void PhysicsTick(float deltaTime);

    /**
     * Holds the inverse of the body's inertia tensor. The
     * inertia tensor provided must not be degenerate
     * (that would mean the body had zero inertia for
     * spinning along one axis). As long as the tensor is
     * finite, it will be invertible. The inverse tensor
     * is used for similar reasons to the use of inverse
     * mass.
     *
     * The inertia tensor, unlike the other variables that
     * define a rigid body, is given in body space.
     *
     * @see inverseMass
     */
    Matrix3x3 inverseInertiaTensor_{ Matrix3x3::IdentityMatrix };

    /**
     * Holds the inverse inertia tensor of the body in world
     * space. The inverse inertia tensor member is specified in
     * the body's local space.
     *
     * @see inverseInertiaTensor
     */
    Matrix3x3 inverseInertiaTensorWorld_{ Matrix3x3::IdentityMatrix };

    Quaternion objectBaseWorldRotation_{ Quaternion::Identity };
    Quaternion physicsWorldRotation_{ Quaternion::Identity };

    Vector3 eulerRotation_{ Vector3::ZeroVector };

    Vector3 velocity_{ Vector3::ZeroVector };
    Vector3 acceleration_{ Vector3::ZeroVector };

    /**
     * Holds the linear acceleration of the rigid body, for the
     * previous frame.
     */
    Vector3 lastFrameAcceleration_{ Vector3::ZeroVector };

    /**
     * Holds the accumulated force to be applied at the next
     * integration step.
     */
    Vector3 forceAccum_{ Vector3::ZeroVector };

    /**
     * Holds the accumulated torque to be applied at the next
     * integration step.
     */
    Vector3 torqueAccum_{ Vector3::ZeroVector };

    /**
     * Holds the amount of motion of the body. This is a recency
     * weighted mean that can be used to put a body to sleap.
     */
    float motion_{ 0.f };

    // Mass is in kg
    float inverseMass_{ 1.f };
    float linearDamping_{ 0.99f };
    float angularDamping_ { 0.99f };

    float remainingPhysicsTickDeltaTime_{ 0.f };

    bool isGravityEnabled_{ true };

    /**
     * A body can be put to sleep to avoid it being updated
     * by the integration functions or affected by collisions
     * with the world.
     */
    bool isAwake_{ true };

    /**
     * Some bodies may never be allowed to fall asleep.
     * User controlled bodies, for example, should be
     * always awake.
     */
    bool canSleep_{ true };

    bool isPhysicsEnabled_{ true };

    bool isKinematic_{ false };
};

#endif