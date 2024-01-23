#include "pch.h"

#include "ContactResolver.h"
#include "PhysicsContact.h"

ContactResolver::ContactResolver(unsigned int iterations, float velocityEpsilon, float positionEpsilon)
{
    SetIterations(iterations, iterations);
    SetEpsilon(velocityEpsilon, positionEpsilon);
}

ContactResolver::ContactResolver(unsigned int velocityIterations, unsigned int positionIterations, float velocityEpsilon, float positionEpsilon)
{
    SetIterations(velocityIterations, positionIterations);
    SetEpsilon(velocityEpsilon, positionEpsilon);
}

void ContactResolver::ResolveContacts(PhysicsContact* contactArray, unsigned int numContacts, float duration)
{
    // Make sure we have something to do.
    if (numContacts == 0)
    {
        return;
    }

    if (!IsValid())
    {
        return;
    }

    // Prepare the contacts for processing
    PrepareContacts(contactArray, numContacts, duration);

    // Resolve the interpenetration problems with the contacts.
    AdjustPositions(contactArray, numContacts, duration);

    // Resolve the velocity problems with the contacts.
    AdjustVelocities(contactArray, numContacts, duration);
}

void ContactResolver::PrepareContacts(PhysicsContact* contacts, unsigned int numContacts, float duration)
{
    // Generate contact velocity and axis information.
    PhysicsContact* lastContact = contacts + numContacts;
    for (PhysicsContact* contact = contacts; contact < lastContact; contact++)
    {
        // Calculate the internal contact data (inertia, basis, etc).
        contact->CalculateInternals(duration);
    }
}

void ContactResolver::AdjustVelocities(PhysicsContact* contacts, unsigned int numContacts, float duration)
{
    Vector3 velocityChange[2], rotationChange[2];
    Vector3 deltaVel;

    // iteratively handle impacts in order of severity.
    velocityIterationsUsed = 0;
    while (velocityIterationsUsed < velocityIterations_)
    {
        // Find contact with maximum magnitude of probable velocity change.
        float max = velocityEpsilon_;
        unsigned index = numContacts;
        for (unsigned i = 0; i < numContacts; i++)
        {
            if (max < contacts[i].desiredDeltaVelocity)
            {
                max = contacts[i].desiredDeltaVelocity;
                index = i;
            }
        }
        if (index == numContacts)
        {
            break;
        }

        // Match the awake state at the contact
        contacts[index].MatchAwakeState();

        // Do the resolution on the contact that came out top.
        contacts[index].ApplyVelocityChange(velocityChange, rotationChange);

        // With the change in velocity of the two bodies, the update of
        // contact velocities means that some of the relative closing
        // velocities need recomputing.
        for (unsigned int i = 0; i < numContacts; i++)
        {
            // Check each body in the contact
            for (unsigned int b = 0; b < 2; b++)
            {
                if (contacts[i].body[b])
                {
                    // Check for a match with each body in the newly
                    // resolved contact
                    for (unsigned int d = 0; d < 2; d++)
                    {
                        if (contacts[i].body[b] == contacts[index].body[d])
                        {
                            deltaVel = velocityChange[d] + rotationChange[d].Cross(contacts[i].relativeContactPosition[b]);

                            // The sign of the change is negative if we're dealing
                            // with the second body in a contact.
                            Vector3 contactVelocityAddition = contacts[i].contactToWorld.MultiplyTransposeBy(deltaVel) * (b ? -1.f : 1.f);

                            if (contactVelocityAddition.ContainsNanOrInf())
                            {
                                continue;
                            }

                            contacts[i].contactVelocity += contactVelocityAddition;
                            contacts[i].CalculateDesiredDeltaVelocity(duration);
                        }
                    }
                }
            }
        }
        velocityIterationsUsed++;
    }
}

void ContactResolver::AdjustPositions(PhysicsContact* contacts, unsigned int numContacts, float duration)
{
    unsigned int i, index;
    Vector3 linearChange[2], angularChange[2];
    float max;
    Vector3 deltaPosition;

    // iteratively resolve interpenetrations in order of severity.
    positionIterationsUsed = 0;
    while (positionIterationsUsed < positionIterations_)
    {
        // Find biggest penetration
        max = positionEpsilon_;
        index = numContacts;
        for (i = 0; i < numContacts; i++)
        {
            if (max < contacts[i].penetration)
            {
                max = contacts[i].penetration;
                index = i;
            }
        }

        if (index == numContacts)
        {
            break;
        }

        // Match the awake state at the contact
        contacts[index].MatchAwakeState();

        // Resolve the penetration.
        contacts[index].ApplyPositionChange(linearChange, angularChange, max);

        // Again this action may have changed the penetration of other
        // bodies, so we update contacts.
        for (i = 0; i < numContacts; i++)
        {
            // Check each body in the contact
            for (unsigned b = 0; b < 2; b++)
            {
                if (contacts[i].body[b])
                {
                    // Check for a match with each body in the newly
                    // resolved contact
                    for (unsigned d = 0; d < 2; d++)
                    {
                        if (contacts[i].body[b] == contacts[index].body[d])
                        {
                            deltaPosition = linearChange[d] + angularChange[d].Cross(contacts[i].relativeContactPosition[b]);

                            // The sign of the change is positive if we're
                            // dealing with the second body in a contact
                            // and negative otherwise (because we're
                            // subtracting the resolution)..
                            contacts[i].penetration += deltaPosition.Dot(contacts[i].contactNormal) * (b ? 1 : -1);
                        }
                    }
                }
            }
        }
        positionIterationsUsed++;
    }
}
