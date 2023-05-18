#include "pch.h"

#include "PhysicsContact.h"

#include "Log.h"
#include "Physics/RigidBody.h"

void PhysicsContact::SetBodyData(RigidBody* one, RigidBody* two, float friction, float restitution)
{
    body[0] = one;
    body[1] = two;
    this->friction = friction;
    this->restitution = restitution;
}

void PhysicsContact::MatchAwakeState()
{
    // Collisions with the world never cause a body to wake up.
    if (!body[1]) return;

    bool body0awake = body[0]->GetIsAwake();
    bool body1awake = body[1]->GetIsAwake();

    // Wake up only the sleeping one
    if (body0awake ^ body1awake)
    {
        if (body0awake)
        {
            body[1]->SetIsAwake();
        }
        else
        {
            body[0]->SetIsAwake();
        }
    }
}

/*
 * Swaps the bodies in the current contact, so body 0 is at body 1 and
 * vice versa. This also changes the direction of the contact normal,
 * but doesn't update any calculated internal data. If you are calling
 * this method manually, then call calculateInternals afterwards to
 * make sure the internal data is up to date.
 */
void PhysicsContact::SwapBodies()
{
    contactNormal *= -1;

    RigidBody* temp = body[0];
    body[0] = body[1];
    body[1] = temp;
}

/*
 * Constructs an arbitrary orthonormal basis for the contact.  This is
 * stored as a 3x3 matrix, where each vector is a column (in other
 * words the matrix transforms contact space into world space). The x
 * direction is generated from the contact normal, and the y and z
 * directionss are set so they are at right angles to it.
 */
inline void PhysicsContact::CalculateContactBasis()
{
    Vector3 contactTangent[2];

    // Check whether the Z-axis is nearer to the X or Y axis
    if (std::abs(contactNormal.x) > std::abs(contactNormal.y))
    {
        // Scaling factor to ensure the results are normalised
        const float s = 1.0f / std::sqrtf(contactNormal.z * contactNormal.z + contactNormal.x * contactNormal.x);

        // The new X-axis is at right angles to the world Y-axis
        contactTangent[0].x = contactNormal.z * s;
        contactTangent[0].y = 0;
        contactTangent[0].z = -contactNormal.x * s;

        // The new Y-axis is at right angles to the new X- and Z- axes
        contactTangent[1].x = contactNormal.y * contactTangent[0].x;
        contactTangent[1].y = contactNormal.z * contactTangent[0].x - contactNormal.x * contactTangent[0].z;
        contactTangent[1].z = -contactNormal.y * contactTangent[0].x;
    }
    else
    {
        // Scaling factor to ensure the results are normalised
        const float s = 1.0f / std::sqrtf(contactNormal.z * contactNormal.z + contactNormal.y * contactNormal.y);

        // The new X-axis is at right angles to the world X-axis
        contactTangent[0].x = 0;
        contactTangent[0].y = -contactNormal.z * s;
        contactTangent[0].z = contactNormal.y * s;

        // The new Y-axis is at right angles to the new X- and Z- axes
        contactTangent[1].x = contactNormal.y * contactTangent[0].z - contactNormal.z * contactTangent[0].y;
        contactTangent[1].y = -contactNormal.x * contactTangent[0].z;
        contactTangent[1].z = contactNormal.x * contactTangent[0].y;
    }

    // Make a matrix from the three vectors.
    contactToWorld = Matrix3x3(
        contactNormal.x, contactNormal.y, contactNormal.z, 
        contactTangent[0].x, contactTangent[0].y, contactTangent[0].z, 
        contactTangent[1].x, contactTangent[1].y, contactTangent[1].z);
}

void PhysicsContact::ApplyImpulse(const Vector3& impulse, RigidBody* body, Vector3* velocityChange, Vector3* rotationChange)
{
}

Vector3 PhysicsContact::CalculateLocalVelocity(unsigned int bodyIndex, float duration)
{
    RigidBody* thisBody = body[bodyIndex];

    // Work out the velocity of the contact point.
    Vector3 velocity = thisBody->GetWorldRotation().ToEuler().Cross(relativeContactPosition[bodyIndex]);
    velocity += thisBody->GetVelocity();

    // Turn the velocity into contact-coordinates.
    Vector3 contactVelocity = contactToWorld.MultiplyTransposeBy(velocity);

    // Calculate the ammount of velocity that is due to forces without reactions.
    Vector3 accVelocity = thisBody->GetLastFrameAcceleration() * duration;

    // Calculate the velocity in contact-coordinates.
    accVelocity = contactToWorld.MultiplyTransposeBy(accVelocity);

    // We ignore any component of acceleration in the contact normal
    // direction, we are only interested in planar acceleration
    accVelocity.x = 0;

    // Add the planar velocities - if there's enough friction they will
    // be removed during velocity resolution
    contactVelocity += accVelocity;

    // And return it
    return contactVelocity;
}


void PhysicsContact::CalculateDesiredDeltaVelocity(float duration)
{
    const static float velocityLimit = 0.25f;

    // Calculate the acceleration induced velocity accumulated this frame
    float velocityFromAcc = 0.f;

    if (body[0]->GetIsAwake())
    {
        velocityFromAcc += (body[0]->GetLastFrameAcceleration() * duration).Dot(contactNormal);
    }

    if (body[1] && body[1]->GetIsAwake())
    {
        velocityFromAcc -=  (body[1]->GetLastFrameAcceleration() * duration).Dot(contactNormal);
    }

    // If the velocity is very slow, limit the restitution
    float thisRestitution = restitution;
    if (std::abs(contactVelocity.x) < velocityLimit)
    {
        thisRestitution = 0.f;
    }

    // Combine the bounce velocity with the removed
    // acceleration velocity.
    desiredDeltaVelocity = -contactVelocity.x - thisRestitution * (contactVelocity.x - velocityFromAcc);
}


void PhysicsContact::CalculateInternals(float duration)
{
    // Check if the first object is NULL, and swap if it is.
    if (!body[0])
    {
        SwapBodies();
    }
    GOKNAR_CORE_ASSERT(body[0]);

    // Calculate an set of axis at the contact point.
    CalculateContactBasis();

    // Store the relative position of the contact relative to each body
    relativeContactPosition[0] = contactPoint - body[0]->GetWorldPosition();
    if (body[1])
    {
        relativeContactPosition[1] = contactPoint - body[1]->GetWorldPosition();
    }

    // Find the relative velocity of the bodies at the contact point.
    contactVelocity = CalculateLocalVelocity(0, duration);
    if (body[1])
    {
        contactVelocity -= CalculateLocalVelocity(1, duration);
    }

    // Calculate the desired change in velocity for resolution
    CalculateDesiredDeltaVelocity(duration);
}

void PhysicsContact::ApplyVelocityChange(Vector3 velocityChange[2], Vector3 rotationChange[2])
{
    // Get hold of the inverse mass and inverse inertia tensor, both in
    // world coordinates.
    Matrix3x3 inverseInertiaTensor[2];
    body[0]->GetInverseInertiaTensorWorld(&inverseInertiaTensor[0]);
    if (body[1])
    {
        body[1]->GetInverseInertiaTensorWorld(&inverseInertiaTensor[1]);
    }

    // We will calculate the impulse for each contact axis
    Vector3 impulseContact;

    if (friction == 0.f)
    {
        // Use the short format for friction_less contacts
        impulseContact = CalculateFrictionlessImpulse(inverseInertiaTensor);
    }
    else
    {
        // Otherwise we may have impulses that aren't in the direction of the
        // contact, so we need the more complex version.
        impulseContact = CalculateFrictionImpulse(inverseInertiaTensor);
    }

    // Convert impulse to world coordinates
    Vector3 impulse = contactToWorld * impulseContact;

    // Split in the impulse into linear and rotational components
    Vector3 impulsiveTorque = relativeContactPosition[0].Cross(impulse);
    rotationChange[0] = inverseInertiaTensor[0] * impulsiveTorque;
    velocityChange[0] = Vector3::ZeroVector;
    velocityChange[0] += impulse * body[0]->GetInverseMass();

    // Apply the changes
    body[0]->AddVelocity(velocityChange[0]);
    body[0]->SetWorldRotation(body[0]->GetWorldRotation() + Quaternion::FromEuler(rotationChange[0]));

    if (body[1])
    {
        // Work out body one's linear and angular changes
        Vector3 impulsiveTorque = impulse.Cross(relativeContactPosition[1]);
        rotationChange[1] = inverseInertiaTensor[1] * impulsiveTorque;
        velocityChange[1] = Vector3::ZeroVector;
        velocityChange[1] += impulse * -body[1]->GetInverseMass();

        // And apply them.
        body[1]->AddVelocity(velocityChange[1]);
        body[1]->SetWorldRotation(body[0]->GetWorldRotation() + Quaternion::FromEuler(rotationChange[1]));
    }
}

inline Vector3 PhysicsContact::CalculateFrictionlessImpulse(Matrix3x3* inverseInertiaTensor)
{
    Vector3 impulseContact;

    // Build a vector that shows the change in velocity in
    // world space for a unit impulse in the direction of the contact
    // normal.
    Vector3 deltaVelWorld = relativeContactPosition[0].Cross(contactNormal);
    deltaVelWorld = inverseInertiaTensor[0] * deltaVelWorld;
    deltaVelWorld = deltaVelWorld.Cross(relativeContactPosition[0]);

    // Work out the change in velocity in contact coordiantes.
    float deltaVelocity = deltaVelWorld.Dot(contactNormal);

    // Add the linear component of velocity change
    deltaVelocity += body[0]->GetInverseMass();

    // Check if we need to the second body's data
    if (body[1])
    {
        // Go through the same transformation sequence again
        Vector3 deltaVelWorld = relativeContactPosition[1].Cross(contactNormal);
        deltaVelWorld = inverseInertiaTensor[1] * deltaVelWorld;
        deltaVelWorld = deltaVelWorld.Cross(relativeContactPosition[1]);

        // Add the change in velocity due to rotation
        deltaVelocity += deltaVelWorld.Dot(contactNormal);

        // Add the change in velocity due to linear motion
        deltaVelocity += body[1]->GetInverseMass();
    }

    // Calculate the required size of the impulse
    impulseContact.x = desiredDeltaVelocity / deltaVelocity;
    impulseContact.y = 0;
    impulseContact.z = 0;
    return impulseContact;
}

inline Vector3 PhysicsContact::CalculateFrictionImpulse(Matrix3x3* inverseInertiaTensor)
{
    Vector3 impulseContact;
    float inverseMass = body[0]->GetInverseMass();

    // The equivalent of a cross product in matrices is multiplication
    // by a skew symmetric matrix - we build the matrix for converting
    // between linear and angular quantities.
    Matrix3x3 impulseToTorque;
    impulseToTorque.SetSkewSymmetric(relativeContactPosition[0]);

    // Build the matrix to convert contact impulse to change in velocity
    // in world coordinates.
    Matrix3x3 deltaVelWorld = impulseToTorque;
    deltaVelWorld *= inverseInertiaTensor[0];
    deltaVelWorld *= impulseToTorque;
    deltaVelWorld *= -1;

    // Check if we need to add body two's data
    if (body[1])
    {
        // Set the cross product matrix
        impulseToTorque.SetSkewSymmetric(relativeContactPosition[1]);

        // Calculate the velocity change matrix
        Matrix3x3 deltaVelWorld2 = impulseToTorque;
        deltaVelWorld2 *= inverseInertiaTensor[1];
        deltaVelWorld2 *= impulseToTorque;
        deltaVelWorld2 *= -1;

        // Add to the total delta velocity.
        deltaVelWorld += deltaVelWorld2;

        // Add to the inverse mass
        inverseMass += body[1]->GetInverseMass();
    }

    // Do a change of basis to convert into contact coordinates.
    Matrix3x3 deltaVelocity = contactToWorld.GetTranspose();
    deltaVelocity *= deltaVelWorld;
    deltaVelocity *= contactToWorld;

    // Add in the linear velocity change
    deltaVelocity.m[0] += inverseMass;
    deltaVelocity.m[4] += inverseMass;
    deltaVelocity.m[8] += inverseMass;

    // Invert to get the impulse needed per unit velocity
    Matrix3x3 impulseMatrix;
    deltaVelocity.GetInverse(impulseMatrix);

    // Find the target velocities to kill
    Vector3 velKill(desiredDeltaVelocity, -contactVelocity.y, -contactVelocity.z);

    // Find the impulse to kill target velocities
    impulseContact = impulseMatrix * velKill;

    // Check for exceeding friction
    float planarImpulse = std::sqrtf(impulseContact.y * impulseContact.y + impulseContact.z * impulseContact.z
    );
    if (planarImpulse > impulseContact.x * friction)
    {
        // We need to use dynamic friction
        impulseContact.y /= planarImpulse;
        impulseContact.z /= planarImpulse;

        impulseContact.x = deltaVelocity.m[0] + deltaVelocity.m[1] * friction * impulseContact.y + deltaVelocity.m[2] * friction * impulseContact.z;
        impulseContact.x = desiredDeltaVelocity / impulseContact.x;
        impulseContact.y *= friction * impulseContact.x;
        impulseContact.z *= friction * impulseContact.x;
    }
    return impulseContact;
}

void PhysicsContact::ApplyPositionChange(Vector3 linearChange[2], Vector3 angularChange[2], float penetration)
{
    const float angularLimit = 0.2f;
    float angularMove[2];
    float linearMove[2];

    float totalInertia = 0;
    float linearInertia[2];
    float angularInertia[2];

    // We need to work out the inertia of each object in the direction
    // of the contact normal, due to angular inertia only.
    for (unsigned i = 0; i < 2; i++)
    {
        if (body[i])
        {
            Matrix3x3 inverseInertiaTensor;
            body[i]->GetInverseInertiaTensorWorld(&inverseInertiaTensor);

            // Use the same procedure as for calculating friction_less
            // velocity change to work out the angular inertia.
            Vector3 angularInertiaWorld = relativeContactPosition[i].Cross(contactNormal);
            angularInertiaWorld = inverseInertiaTensor * angularInertiaWorld;
            angularInertiaWorld = angularInertiaWorld.Cross(relativeContactPosition[i]);
            angularInertia[i] = angularInertiaWorld.Dot(contactNormal);

            // The linear component is simply the inverse mass
            linearInertia[i] = body[i]->GetInverseMass();

            // Keep track of the total inertia from all components
            totalInertia += linearInertia[i] + angularInertia[i];

            // We break the loop here so that the totalInertia value is
            // completely calculated (by both iterations) before
            // continuing.
        }
    }

    // Loop through again calculating and applying the changes
    for (unsigned i = 0; i < 2; i++)
    {
        if (body[i])
        {
            // The linear and angular movements required are in proportion to
            // the two inverse inertias.
            float sign = (i == 0) ? 1 : -1;
            angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
            linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

            // To avoid angular projections that are too great (when mass is large
            // but inertia tensor is small) limit the angular move.
            Vector3 projection = relativeContactPosition[i];
            projection += (contactNormal, -relativeContactPosition[i].Dot(contactNormal));

            // Use the small angle approximation for the sine of the angle (i.e.
            // the magnitude would be sine(angularLimit) * projection.magnitude
            // but we approximate sine(angularLimit) to angularLimit).
            float maxMagnitude = angularLimit * projection.Length();

            if (angularMove[i] < -maxMagnitude)
            {
                float totalMove = angularMove[i] + linearMove[i];
                angularMove[i] = -maxMagnitude;
                linearMove[i] = totalMove - angularMove[i];
            }
            else if (angularMove[i] > maxMagnitude)
            {
                float totalMove = angularMove[i] + linearMove[i];
                angularMove[i] = maxMagnitude;
                linearMove[i] = totalMove - angularMove[i];
            }

            // We have the linear amount of movement required by turning
            // the rigid body (in angularMove[i]). We now need to
            // calculate the desired rotation to achieve that.
            if (angularMove[i] == 0)
            {
                // Easy case - no angular movement means no rotation.
                angularChange[i] = Vector3::ZeroVector;
            }
            else
            {
                // Work out the direction we'd like to rotate in.
                Vector3 targetAngularDirection = relativeContactPosition[i].Cross(contactNormal);

                Matrix3x3 inverseInertiaTensor;
                body[i]->GetInverseInertiaTensorWorld(&inverseInertiaTensor);

                // Work out the direction we'd need to rotate to achieve that
                angularChange[i] = inverseInertiaTensor * targetAngularDirection * (angularMove[i] / angularInertia[i]);
            }

            // Velocity change is easier - it is just the linear movement
            // along the contact normal.
            linearChange[i] = contactNormal * linearMove[i];

            // Now we can start to apply the values we've calculated.
            // Apply the linear movement
            Vector3 pos = body[i]->GetWorldPosition();
            pos += contactNormal * linearMove[i];
            body[i]->SetWorldPosition(pos);

            // And the change in orientation
            Quaternion q = body[i]->GetWorldRotation();
            q += Quaternion::FromEuler(angularChange[i]);
            body[i]->SetWorldRotation(q);

            // We need to calculate the derived data for any body that is
            // asleep, so that the changes are reflected in the object's
            // data. Otherwise the resolution will not change the position
            // of the object, and the next collision detection round will
            // have the same penetration.
            if (!body[i]->GetIsAwake())
            {
                body[i]->CalculateDerivedData();
            }
        }
    }
}
