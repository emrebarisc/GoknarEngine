#ifndef __FINECOLLISION_H__
#define __FINECOLLISION_H__

#include "Core.h"

#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

class CollisionDetector;
class IntersectionTests;
class RigidBody;
class PhysicsContact;

/**
 * Represents a primitive to detect collisions against.
 */
class GOKNAR_API CollisionPrimitive
{
    friend class IntersectionTests;
    friend class CollisionDetector;

public:
    /**
     * Calculates the internals for the primitive.
     */
    void CalculateInternals();

    /**
     * This is a convenience function to allow access to the
     * axis vectors in the transform for this primitive.
     */
    Vector3 GetAxis(unsigned int index) const
    {
        return transform_.GetAxisVector(index == 0 ? Axis::X : (index == 1 ? Axis::Y : Axis::Z));
    }

    /**
     * Returns the resultant transform of the primitive, calculated from
     * the combined offset of the primitive and the transform
     * (orientation + position) of the rigid body to which it is
     * attached.
     */
    const Matrix& GetTransform() const
    {
        return transform_;
    }

    /**
     * The rigid body that is represented by this primitive.
     */
    RigidBody* body;

    /**
     * The offset of this primitive from the given rigid body.
     */
    Matrix offset;

protected:
    /**
     * The resultant transform of the primitive. This is
     * calculated by combining the offset of the primitive
     * with the transform of the rigid body.
     */
    Matrix transform_;
};

/**
 * Represents a rigid body that can be treated as a sphere
 * for collision detection.
 */
class GOKNAR_API CollisionSphere : public CollisionPrimitive
{
public:
    /**
     * The radius of the sphere.
     */
    float radius;
};

/**
 * The plane is not a primitive: it doesn't represent another
 * rigid body. It is used for contacts with the immovable
 * world geometry.
 */
class GOKNAR_API CollisionPlane
{
public:
    /**
     * The plane normal
     */
    Vector3 direction;

    /**
     * The distance of the plane from the origin.
     */
    float offset;
};

/**
 * Represents a rigid body that can be treated as an aligned bounding
 * box for collision detection.
 */
class GOKNAR_API CollisionBox : public CollisionPrimitive
{
public:
    /**
     * Holds the half-sizes of the box along each of its local axes.
     */
    Vector3 halfSize;
};

/**
 * A wrapper class that holds fast intersection tests. These
 * can be used to drive the coarse collision detection system or
 * as an early out in the full collision tests below.
 */
class GOKNAR_API IntersectionTests
{
public:
    static bool SphereAndHalfSpace(const CollisionSphere& sphere, const CollisionPlane& plane);
    static bool SphereAndSphere(const CollisionSphere& one, const CollisionSphere& two);
    static bool BoxAndBox(const CollisionBox& one, const CollisionBox& two);

    /**
     * Does an intersection test on an arbitrarily aligned box and a
     * half-space.
     *
     * The box is given as a transform matrix, including
     * position, and a vector of half-sizes for the extend of the
     * box along each local axis.
     *
     * The half-space is given as a direction (i.e. unit) vector and the
     * offset of the limiting plane from the origin, along the given
     * direction.
     */
    static bool BoxAndHalfSpace(const CollisionBox& box, const CollisionPlane& plane);
};


/**
 * A helper structure that contains information for the detector to use
 * in building its contact data.
 */
struct GOKNAR_API CollisionData
{
    /**
     * Holds the base of the collision data: the first contact
     * in the array. This is used so that the contact pointer (below)
     * can be incremented each time a contact is detected, while
     * this pointer points to the first contact found.
     */
    PhysicsContact* contactArray;

    /** Holds the contact array to write into. */
    PhysicsContact* contacts;

    /** Holds the maximum number of contacts the array can take. */
    int contactsLeft;

    /** Holds the number of contacts found so far. */
    unsigned int contactCount;

    /** Holds the friction value to write into any collisions. */
    float friction;

    /** Holds the restitution value to write into any collisions. */
    float restitution;

    /**
     * Holds the collision tolerance, even uncolliding objects this
     * close should have collisions generated.
     */
    float tolerance;

    /**
     * Checks if there are more contacts available in the contact
     * data.
     */
    bool HasMoreContacts()
    {
        return contactsLeft > 0;
    }

    /**
     * Resets the data so that it has no used contacts recorded.
     */
    void Reset(unsigned int maxContacts)
    {
        contactsLeft = maxContacts;
        contactCount = 0;
        contacts = contactArray;
    }

    /**
     * Notifies the data that the given number of contacts have
     * been added.
     */
    void AddContacts(unsigned int count);
};

/**
 * A wrapper class that holds the fine grained collision detection
 * routines.
 *
 * Each of the functions has the same format: it takes the details
 * of two objects, and a pointer to a contact array to fill. It
 * returns the number of contacts it wrote into the array.
 */
class GOKNAR_API CollisionDetector
{
public:
    static unsigned int SphereAndHalfSpace(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionData* data);
    static unsigned int SphereAndTruePlane(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionData* data);
    static unsigned int SphereAndSphere(const CollisionSphere& one, const CollisionSphere& two, CollisionData* data);

    /**
     * Does a collision test on a collision box and a plane representing
     * a half-space (i.e. the normal of the plane
     * points out of the half-space).
     */
    static unsigned int BoxAndHalfSpace(const CollisionBox& box, const CollisionPlane& plane, CollisionData* data);
    static unsigned int BoxAndBox(const CollisionBox& one, const CollisionBox& two, CollisionData* data);
    static unsigned int BoxAndPoint(const CollisionBox& box, const Vector3& point, CollisionData* data);
    static unsigned int BoxAndSphere(const CollisionBox& box, const CollisionSphere& sphere, CollisionData* data);
};

#endif