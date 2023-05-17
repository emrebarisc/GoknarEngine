#ifndef __PHYSICSLINKS_H__
#define __PHYSICSLINKS_H__

#include "Core.h"

#include "ContactGenerator.h"
#include "Math/GoknarMath.h"

class RigidBody;

class GOKNAR_API RigidBodyLink : public ContactGenerator
{
public:
    virtual unsigned int AddContact(PhysicsContact *contact, unsigned int limit) const = 0;

    /**
     * Holds the pair of particles that are connected by this link.
     */
    RigidBody* body[2];
protected:
    /**
     * Returns the current length of the link.
     */
    float CurrentLength() const;
};

/**
 * Cables link a pair of particles, generating a contact if they
 * stray too far apart.
 */
class PhysicsCable : public RigidBodyLink
{
public:
    virtual unsigned int AddContact(PhysicsContact *contact, unsigned int limit) const;

    /**
     * Holds the maximum length of the cable.
     */
    float maxLength;

    /**
     * Holds the restitution (bounciness) of the cable.
     */
    float restitution;
};

/**
 * Rods link a pair of particles, generating a contact if they
 * stray too far apart or too close.
 */
class PhysicsRod : public RigidBodyLink
{
public:
    /**
     * Fills the given contact structure with the contact needed
     * to keep the rod from extending or compressing.
     */
    virtual unsigned int AddContact(PhysicsContact* contact, unsigned int limit) const;

    /**
     * Holds the length of the rod.
     */
    float length;
};

/**
* Constraints are just like links, except they connect a particle to
* an immovable anchor point.
*/
class PhysicsConstraint : public ContactGenerator
{
public:
    /**
    * Geneates the contacts to keep this link from being
    * violated. This class can only ever generate a single
    * contact, so the pointer can be a pointer to a single
    * element, the limit parameter is assumed to be at least one
    * (zero isn't valid) and the return value is either 0, if the
    * cable wasn't over-extended, or one if a contact was needed.
    *
    * NB: This method is declared in the same way (as pure
    * virtual) in the parent class, but is replicated here for
    * documentation purposes.
    */
    virtual unsigned int AddContact(PhysicsContact* contact, unsigned int limit) const = 0;

    /**
    * Holds the particles connected by this constraint.
    */
    RigidBody* rigidBody;

    /**
     * The point to which the particle is anchored.
     */
    Vector3 anchor;

protected:
    /**
    * Returns the current length of the link.
    */
    float CurrentLength() const;
};

/**
* Cables link a particle to an anchor point, generating a contact if they
* stray too far apart.
*/
class PhysicsCableConstraint : public PhysicsConstraint
{
public:
    /**
    * Fills the given contact structure with the contact needed
    * to keep the cable from over-extending.
    */
    virtual unsigned int AddContact(PhysicsContact* contact, unsigned int limit) const;

    /**
    * Holds the maximum length of the cable.
    */
    float maxLength;

    /**
    * Holds the restitution (bounciness) of the cable.
    */
    float restitution;
};

/**
* Rods link a particle to an anchor point, generating a contact if they
* stray too far apart or too close.
*/
class PhysicsRodConstraint : public PhysicsConstraint
{
public:
    /**
    * Fills the given contact structure with the contact needed
    * to keep the rod from extending or compressing.
    */
    virtual unsigned int AddContact(PhysicsContact*contact, unsigned int limit) const;

    /**
    * Holds the length of the rod.
    */
    float length;
};

#endif