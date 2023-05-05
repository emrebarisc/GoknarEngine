#ifndef __PHYSICSOBJECTFORCEGENERATOR_H__
#define __PHYSICSOBJECTFORCEGENERATOR_H__

#include "Core.h"

#include "Math/GoknarMath.h"

class PhysicsObject;

class GOKNAR_API PhysicsObjectForceGenerator
{   
public:
    PhysicsObjectForceGenerator();

    /**
     * Overload this in implementations of the interface to calculate
     * and update the force applied to the given physicsObject.
     */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration) = 0;
};

/**
 * A force generator that applies a gravitational force. One instance
 * can be used for multiple physicsObjects.
 */
class PhysicsObjectGravityForceGenerator : public PhysicsObjectForceGenerator
{
public:

    /** Creates the generator with the given acceleration. */
    PhysicsObjectGravityForceGenerator(const Vector3& gravity);

    /** Applies the gravitational force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /** Holds the acceleration due to gravity_. */
    Vector3 gravity_;
};

/**
 * A force generator that applies a drag force. One instance
 * can be used for multiple physicsObjects.
 */
class PhysicsObjectDragForceGenerator : public PhysicsObjectForceGenerator
{
public:
    /** Creates the generator with the given coefficients. */
    PhysicsObjectDragForceGenerator(float k1, float k2) : k1_(k1), k2(k2) {}

    /** Applies the drag force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /** Holds the velocity drag coeffificent. */
    float k1_;

    /** Holds the velocity squared drag coeffificent. */
    float k2;
};

/**
 * A force generator that applies a Spring force, where
 * one end is attached to a fixed point in space.
 */
class PhysicsObjectAnchoredSpringForceGenerator : public PhysicsObjectForceGenerator
{
public:
    PhysicsObjectAnchoredSpringForceGenerator() {}

    /** Creates a new spring with the given parameters. */
    PhysicsObjectAnchoredSpringForceGenerator(Vector3* anchor, float springConstant, float restLength) :
        anchor_(anchor),
        springConstant_(springConstant),
        restLength_(restLength)
    {}

    /** Retrieve the anchor point. */
    const Vector3* GetAnchor() const
    {
        return anchor_;
    }

    /** Set the spring's properties. */
    void Init(Vector3* anchor, float springConstant, float restLength_);

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject *physicsObject, float duration);

protected:
    /** The location of the anchored end of the spring. */
    Vector3* anchor_{ nullptr };

    /** Holds the sprint constant. */
    float springConstant_{ 1.f };

    /** Holds the rest length of the spring. */
    float restLength_{ 100.f };
};

/**
* A force generator that applies a bungee force, where
* one end is attached to a fixed point in space.
*/
class PhysicsObjectAnchoredBungeeForceGenerator : public PhysicsObjectAnchoredSpringForceGenerator
{
public:
    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject *physicsObject, float duration);
};

/**
 * A force generator that fakes a stiff spring force, and where
 * one end is attached to a fixed point in space.
 */
class PhysicsObjectFakeSpringForceGenerator : public PhysicsObjectForceGenerator
{
public:
    /** Creates a new spring with the given parameters. */
    PhysicsObjectFakeSpringForceGenerator(Vector3* anchor, float springConstant, float damping) :
        anchor_(anchor),
        springConstant_(springConstant),
        damping_(damping)
    {}

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /** The location of the anchored end of the spring. */
    Vector3* anchor_;

    /** Holds the sprint constant. */
    float springConstant_;

    /** Holds the damping on the oscillation of the spring. */
    float damping_;
};

/**
 * A force generator that applies a Spring force.
 */
class PhysicsObjectSpringForceGenerator : public PhysicsObjectForceGenerator
{
public:
    /** Creates a new spring with the given parameters. */
    PhysicsObjectSpringForceGenerator(PhysicsObject* other, float springConstant, float restLength) :
        other_(other),
        springConstant_(springConstant),
        restLength_(restLength) 
    {}

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /** The physicsObject at the other_ end of the spring. */
    PhysicsObject* other_;

    /** Holds the sprint constant. */
    float springConstant_;

    /** Holds the rest length of the spring. */
    float restLength_;
};

/**
 * A force generator that applies a spring force only
 * when extended.
 */
class PhysicsObjectBungeeForceGenerator : public PhysicsObjectForceGenerator
{
public:
    /** Creates a new bungee with the given parameters. */
    PhysicsObjectBungeeForceGenerator(PhysicsObject* other, float springConstant, float restLength) :
        other_(other),
        springConstant_(springConstant),
        restLength_(restLength)
    {
    }

    /** Applies the spring force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /** The physicsObject at the other_ end of the spring. */
    PhysicsObject* other_;

    /** Holds the sprint constant. */
    float springConstant_;

    /**
    * Holds the length of the bungee at the point it begins to
    * generator a force.
    */
    float restLength_;
};

/**
 * A force generator that applies a buoyancy force for a plane of
 * liquid parrallel to XZ plane.
 */
class PhysicsObjectBuoyancyForceGenerator : public PhysicsObjectForceGenerator
{
public:
    /** Creates a new buoyancy force with the given parameters. */
    PhysicsObjectBuoyancyForceGenerator(float maxDepth, float volume, float waterHeight, float liquidDensity = 1000.0f) :
        maxDepth_(maxDepth),
        volume_(volume),
        waterHeight_(waterHeight),
        liquidDensity_(liquidDensity)
    {}

    /** Applies the buoyancy force to the given physicsObject. */
    virtual void UpdateForce(PhysicsObject* physicsObject, float duration);

private:
    /**
     * The maximum submersion depth of the object before
     * it generates its maximum boyancy force.
     */
    float maxDepth_;

    /**
     * The volume of the object.
     */
    float volume_;

    /**
     * The height of the water plane above y=0. The plane will be
     * parrallel to the XZ plane.
     */
    float waterHeight_;

    /**
     * The density of the liquid. Pure water has a density of
     * 1000kg per cubic meter.
     */
    float liquidDensity_;
};

/**
 * Holds all the force generators and the physicsObjects they apply to.
 */
class PhysicsObjectForceRegistry
{
public:
    /**
     * Registers the given force generator to apply to the
     * given physicsObject.
     */
    void Add(PhysicsObject* physicsObject, PhysicsObjectForceGenerator* forceGenerator);

    /**
     * Removes the given registered pair from the registry.
     * If the pair is not registered, this method will have
     * no effect.
     */
    void Remove(PhysicsObject* physicsObject, PhysicsObjectForceGenerator* forceGenerator);

    /**
     * Clears all registrations from the registry. This will
     * not delete the physicsObjects or the force generators
     * themselves, just the records of their connection.
     */
    void Clear();

    /**
     * Calls all the force generators to update the forces of
     * their corresponding physicsObjects.
     */
    void UpdateForces(float duration);

protected:
    /**
     * Keeps track of one force generator and the physicsObject it
     * applies to.
     */
    struct PhysicsObjectForceRegistration
    {
        PhysicsObject* physicsObject;
        PhysicsObjectForceGenerator* forceGenerator;
    };

    /**
     * Holds the list of registrations.
     */
    typedef std::vector<PhysicsObjectForceRegistration> Registry;
    Registry registrations_;

};

#endif