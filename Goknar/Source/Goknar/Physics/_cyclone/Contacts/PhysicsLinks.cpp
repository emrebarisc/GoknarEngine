#include "pch.h"

#include "PhysicsLinks.h"
#include "PhysicsContact.h"
#include "Physics/RigidBody.h"

float RigidBodyLink::CurrentLength() const
{
    Vector3 relativePos = body[0]->GetWorldPosition() - body[1]->GetWorldPosition();
    return relativePos.Length();
}

unsigned int PhysicsCable::AddContact(PhysicsContact* contact, unsigned int limit) const
{
    // Find the length of the cable
    float length = CurrentLength();

    // Check if we're over-extended
    if (length < maxLength)
    {
        return 0;
    }

    // Otherwise return the contact
    contact->body[0] = body[0];
    contact->body[1] = body[1];

    // Calculate the normal
    Vector3 normal = body[1]->GetWorldPosition() - body[0]->GetWorldPosition();
    normal.Normalize();
    contact->contactNormal = normal;

    contact->penetration = length - maxLength;
    contact->restitution = restitution;

    return 1;
}

unsigned int PhysicsRod::AddContact(PhysicsContact* contact, unsigned int limit) const
{
    // Find the length of the rod
    float currentLen = CurrentLength();

    // Check if we're over-extended
    if (currentLen == length)
    {
        return 0;
    }

    // Otherwise return the contact
    contact->body[0] = body[0];
    contact->body[1] = body[1];

    // Calculate the normal
    Vector3 normal = body[1]->GetWorldPosition() - body[0]->GetWorldPosition();
    normal.Normalize();

    // The contact normal depends on whether we're extending or compressing
    if (length < currentLen)
    {
        contact->contactNormal = normal;
        contact->penetration = currentLen - length;
    }
    else
    {
        contact->contactNormal = normal * -1.f;
        contact->penetration = length - currentLen;
    }

    // Always use zero restitution (no bounciness)
    contact->restitution = 0;

    return 1;
}

float PhysicsConstraint::CurrentLength() const
{
    Vector3 relativePos = rigidBody->GetWorldPosition() - anchor;
    return relativePos.Length();
}

unsigned int PhysicsCableConstraint::AddContact(PhysicsContact* contact, unsigned int limit) const
{
    // Find the length of the cable
    float length = CurrentLength();

    // Check if we're over-extended
    if (length < maxLength)
    {
        return 0;
    }

    // Otherwise return the contact
    contact->body[0] = rigidBody;
    contact->body[1] = 0;

    // Calculate the normal
    Vector3 normal = anchor - rigidBody->GetWorldPosition();
    normal.Normalize();
    contact->contactNormal = normal;

    contact->penetration = length - maxLength;
    contact->restitution = restitution;

    return 1;
}

unsigned int PhysicsRodConstraint::AddContact(PhysicsContact* contact, unsigned int limit) const
{
    // Find the length of the rod
    float currentLen = CurrentLength();

    // Check if we're over-extended
    if (currentLen == length)
    {
        return 0;
    }

    // Otherwise return the contact
    contact->body[0] = rigidBody;
    contact->body[1] = 0;

    // Calculate the normal
    Vector3 normal = anchor - rigidBody->GetWorldPosition();
    normal.Normalize();

    // The contact normal depends on whether we're extending or compressing
    if (currentLen > length)
    {
        contact->contactNormal = normal;
        contact->penetration = currentLen - length;
    }
    else
    {
        contact->contactNormal = normal * -1;
        contact->penetration = length - currentLen;
    }

    // Always use zero restitution (no bounciness)
    contact->restitution = 0;

    return 1;
}