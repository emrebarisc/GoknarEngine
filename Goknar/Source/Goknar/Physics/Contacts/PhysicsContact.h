#ifndef __PHYSICSCONTACT_H__
#define __PHYSICSCONTACT_H__

#include "Goknar/Core.h"

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

class ContactResolver;
class RigidBody;

class GOKNAR_API PhysicsContact
{
    friend class ContactResolver;
public:

protected:
    /**
     * Calculates internal data from state data. This is called before
     * the resolution algorithm tries to do any resolution. It should
     * never need to be called manually.
     */
    void CalculateInternals(float duration);

    /**
     * Reverses the contact. This involves swapping the two rigid bodies
     * and reversing the contact normal. The internal values should then
     * be recalculated using calculateInternals (this is not done
     * automatically).
     */
    void SwapBodies();

    /**
     * Updates the awake state of rigid bodies that are taking
     * place in the given contact. A body will be made awake if it
     * is in contact with a body that is awake.
     */
    void MatchAwakeState();

    /**
     * Calculates and sets the internal value for the desired delta
     * velocity.
     */
    void CalculateDesiredDeltaVelocity(float duration);

    /**
     * Calculates and returns the velocity of the contact
     * point on the given body.
     */
    Vector3 CalculateLocalVelocity(unsigned int bodyIndex, float duration);

    /**
     * Calculates an orthonormal basis for the contact point, based on
     * the primary friction direction (for anisotropic friction) or
     * a random orientation (for isotropic friction).
     */
    void CalculateContactBasis();

    /**
     * Applies an impulse to the given body, returning the change in velocities.
     */
    void ApplyImpulse(const Vector3& impulse, RigidBody* body, Vector3* velocityChange, Vector3* rotationChange);

    /**
     * Performs an inertia-weighted impulse based resolution of this contact alone.
     */
    void ApplyVelocityChange(Vector3 velocityChange[2], Vector3 rotationChange[2]);

    /**
     * Performs an inertia weighted penetration resolution of this contact alone.
     */
    void ApplyPositionChange(Vector3 linearChange[2], Vector3 angularChange[2], float penetration);

    /**
     * Calculates the impulse needed to resolve this contact,
     * given that the contact has no friction. A pair of inertia
     * tensors - one for each contact object - is specified to
     * save calculation time: the calling function has access to
     * these anyway.
     */
    Vector3 CalculateFrictionlessImpulse(Matrix3x3* inverseInertiaTensor);

    /**
     * Calculates the impulse needed to resolve this contact,
     * given that the contact has a non-zero coefficient of
     * friction. A pair of inertia tensors - one for each contact
     * object - is specified to save calculation time: the calling
     * function has access to these anyway.
     */
    Vector3 CalculateFrictionImpulse(Matrix3x3* inverseInertiaTensor);

private:
    /**
        * Sets the data that doesn't normally depend on the position
        * of the contact (i.e. the bodies, and their material properties).
        */
    void SetBodyData(RigidBody* one, RigidBody* two, float friction, float restitution);

    /**
        * Holds the bodies that are involved in the contact. The
        * second of these can be NULL, for contacts with the scenery.
        */
    RigidBody* body_[2];

    /**
        * A transform matrix that converts co-ordinates in the contact's
        * frame of reference to world co-ordinates. The columns of this
        * matrix form an orthonormal set of vectors.
        */
    Matrix3x3 contactToWorld_;

    /**
        * Holds the world space position of the contact point relative to
        * centre of each body. This is set when the calculateInternals
        * function is run.
        */
    Vector3 relativeContactPosition_[2];

    /**
        * Holds the closing velocity at the point of contact. This is set
        * when the calculateInternals function is run.
        */
    Vector3 contactVelocity_;

    /**
        * Holds the position of the contact in world coordinates.
        */
    Vector3 contactPoint_;

    /**
        * Holds the direction of the contact in world coordinates.
        */
    Vector3 contactNormal_;

    /**
        * Holds the lateral friction coefficient at the contact.
        */
    float friction_;

    /**
        * Holds the normal restitution coefficient at the contact.
        */
    float restitution_;

    /**
        * Holds the depth of penetration at the contact point. If both
        * bodies are specified then the contact point should be midway
        * between the inter-penetrating points.
        */
    float penetration_;

    /**
        * Holds the required change in velocity for this contact to be
        * resolved.
        */
    float desiredDeltaVelocity_;

};

#endif