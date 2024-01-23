#ifndef __FORCEGENERATOR_H__
#define __FORCEGENERATOR_H__

#include "Core.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

#include "ForceGenerator.h"

class RigidBody;

class GOKNAR_API ForceGenerator
{   
public:
    ForceGenerator();

    virtual void UpdateForce(RigidBody* rigidBody, float duration) = 0;
};

class Gravity : public ForceGenerator
{
public:
    Gravity(const Vector3& gravity) : 
        gravity_(gravity)
    {}

    virtual void UpdateForce(RigidBody* rigidBody, float duration);

private:
    Vector3 gravity_;
};

/**
 * A force generator that applies a drag force. One instance
 * can be used for multiple physicsObjects.
 */
class DragForceGenerator : public ForceGenerator
{
public:
    /** Creates the generator with the given coefficients. */
    DragForceGenerator(float k1, float k2) : k1_(k1), k2_(k2) {}

    /** Applies the drag force to the given physicsObject. */
    virtual void UpdateForce(RigidBody* rigidBody, float deltaTime);

private:
    /** Holds the velocity drag coeffificent. */
    float k1_;

    /** Holds the velocity squared drag coeffificent. */
    float k2_;
};

/**
 * A force generator that applies a Spring force.
 */
class Spring : public ForceGenerator
{
public:
    Spring(const Vector3& localConnectionPoint, RigidBody* other, const Vector3& otherConnectionPoint, float springConstant, float restLength) :
        connectionPoint_(localConnectionPoint),
        other_(other),
        otherConnectionPoint_(otherConnectionPoint),
        springConstant_(springConstant),
        restLength_(restLength)
    {}

    virtual void UpdateForce(RigidBody* rigidBody, float duration);

private:
    // The point of connection of the spring, in local coordinates.
    Vector3 connectionPoint_;

    // The point of connection of the spring to the other object, in that object's local coordinates.
    Vector3 otherConnectionPoint_;

    RigidBody* other_;
    float springConstant_;
    float restLength_;
};

/**
 * A force generator that applies a Spring force, where
 * one end is attached to a fixed point in space.
 */
class AnchoredSpringForceGenerator : public ForceGenerator
{
public:
    /** Creates a new spring with the given parameters. */
    AnchoredSpringForceGenerator(Vector3* anchor, const Vector3& localConnectionPoint, float springConstant, float restLength) :
        anchor_(anchor),
        connectionPoint_(localConnectionPoint),
        springConstant_(springConstant),
        restLength_(restLength)
    {}

    /** Retrieve the anchor point. */
    const Vector3* GetAnchor() const
    {
        return anchor_;
    }

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(RigidBody* rigidBody, float duration);

protected:
    // The point of connection of the spring, in local coordinates.
    Vector3 connectionPoint_;

    /** The location of the anchored end of the spring. */
    Vector3* anchor_{ nullptr };

    /** Holds the sprint constant. */
    float springConstant_{ 1.f };

    /** Holds the rest length of the spring. */
    float restLength_{ 100.f };
};
/**
 * A force generator that applies a spring force only
 * when extended.
 */
class BungeeForceGenerator : public ForceGenerator
{
public:
    BungeeForceGenerator(RigidBody* other, float springConstant, float restLength) :
        other_(other),
        springConstant_(springConstant),
        restLength_(restLength)
    {
    }

    virtual void UpdateForce(RigidBody* rigidBody, float duration);

private:
    RigidBody* other_;
    float springConstant_;
    float restLength_;
};

/**
* A force generator that applies a bungee force, where
* one end is attached to a fixed point in space.
*/
class AnchoredBungeeForceGenerator : public AnchoredSpringForceGenerator
{
public:
    /** Creates a new spring with the given parameters. */
    AnchoredBungeeForceGenerator(Vector3* anchor, const Vector3& localConnectionPoint, float springConstant, float restLength) :
        AnchoredSpringForceGenerator(anchor, localConnectionPoint, springConstant, restLength)
    {}

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(RigidBody* rigidBody, float duration);
};

class Aero : public ForceGenerator
{
public:
    Aero(const Matrix3x3& tensor, const Vector3& position, const Vector3* windSpeed) :
        tensor_(tensor),
        position_(position),
        windSpeed_(windSpeed)
    {}

    virtual void UpdateForce(RigidBody* rigidBody, float deltaTime);

protected:
    /**
     * Uses an explicit tensor matrix to update the force on
     * the given rigid body. This is exactly the same as for UpdateForce
     * only it takes an explicit tensor.
     */
    void UpdateForceFromTensor(RigidBody* rigidBody, float deltaTime, const Matrix3x3& tensor);

    /**
     * Holds the aerodynamic tensor for the surface in body
     * space.
     */
    Matrix3x3 tensor_;

    /**
     * Holds the relative position of the aerodynamic surface in
     * body coordinates.
     */
    Vector3 position_;

    /**
     * Holds a pointer to a vector containing the windspeed of the
     * environment. This is easier than managing a separate
     * windspeed vector per generator and having to update it
     * manually as the wind changes.
     */
    const Vector3* windSpeed_;
};

/**
* A force generator with a control aerodynamic surface. This
* requires three inertia tensors, for the two extremes and
* 'resting' position of the control surface.  The latter tensor is
* the one inherited from the base class, the two extremes are
* defined in this class.
*/
class AeroControl : public Aero
{
public:
    AeroControl(const Matrix3x3& base, const Matrix3x3& min, const Matrix3x3& max, const Vector3& position, const Vector3* windSpeed) :
        Aero(base, position, windSpeed),
        minTensor_(min),
        maxTensor_(max),
        controlSetting_(0.f)
    {}

    /*
     * Sets the control position of this control. This 
     * should range between -1 (in which case the minTensor value is
     * used), through 0 (where the base-class tensor value is used)
     * to +1 (where the maxTensor value is used). Values outside that
     * range give undefined results.
    */
    void SetControl(float value)
    {
        controlSetting_ = value;
    }

    virtual void UpdateForce(RigidBody* rigidBody, float deltaTime);

protected:
    /**
     * The aerodynamic tensor for the surface, when the control is at
     * its maximum value.
     */
    Matrix3x3 maxTensor_;

    /**
     * The aerodynamic tensor for the surface, when the control is at
     * its minimum value.
     */
    Matrix3x3 minTensor_;

    /**
    * The current position of the control for this surface. This
    * should range between -1 (in which case the minTensor value
    * is used), through 0 (where the base-class tensor value is
    * used) to +1 (where the maxTensor value is used).
    */
    float controlSetting_;

private:
    /**
     * Calculates the final aerodynamic tensor for the current
     * control setting.
     */
    Matrix3x3 GetTensor();
};

/**
 * A force generator with an aerodynamic surface that can be
 * re-oriented relative to its rigid body.
 */
class AngledAero : public Aero
{
public:
    AngledAero(const Matrix3x3& tensor, const Vector3& position, const Vector3* windSpeed) :
        Aero(tensor, position, windSpeed)
    {}

    void SetOrientation(const Quaternion& orientation)
    {
        orientation_ = orientation;
    }

    virtual void UpdateForce(RigidBody* rigidBody, float duration);

private:
    Quaternion orientation_;
};

class Buoyancy : public ForceGenerator
{
public:
    Buoyancy(const Vector3& centerOfBouyancy, float maxDepth, float volume, float waterHeight, float liquidDensity = 1000.0f) :
        centerOfBouyancy_(centerOfBouyancy),
        maxDepth_(maxDepth),
        volume_(volume),
        waterHeight_(waterHeight),
        liquidDensity_(liquidDensity)
    {}

    virtual void UpdateForce(RigidBody* rigidBody, float duration);

private:
    float maxDepth_;
    float volume_;
    float waterHeight_;
    float liquidDensity_;
    Vector3 centerOfBouyancy_;
};

class ForceRegistry
{
public:
    ~ForceRegistry();

    void Add(RigidBody* rigidBody, ForceGenerator* forceGenerator);
    void Remove(RigidBody* rigidBody, ForceGenerator* forceGenerator);
    void Clear();

    void UpdateForces(float duration);

protected:
    struct ForceRegistration
    {
        RigidBody* rigidBody;
        ForceGenerator* forceGenerator;
    };

    typedef std::vector<ForceRegistration> Registry;
    Registry registrations_;

};

#endif