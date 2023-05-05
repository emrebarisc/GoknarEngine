#include "pch.h"

#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/ForceGenerators/PhysicsObjectForceGenerator.h"

PhysicsObjectForceGenerator::PhysicsObjectForceGenerator()
{
	
}

void PhysicsObjectForceRegistry::UpdateForces(float duration)
{
    Registry::iterator i = registrations_.begin();
    for (; i != registrations_.end(); i++)
    {
        i->forceGenerator->UpdateForce(i->physicsObject, duration);
    }
}

void PhysicsObjectForceRegistry::Add(PhysicsObject* physicsObject, PhysicsObjectForceGenerator* forceGenerator)
{
    PhysicsObjectForceRegistry::PhysicsObjectForceRegistration registration;
    registration.physicsObject = physicsObject;
    registration.forceGenerator = forceGenerator;
    registrations_.push_back(registration);
}

void PhysicsObjectForceRegistry::Remove(PhysicsObject* physicsObject, PhysicsObjectForceGenerator* forceGenerator)
{
    Registry::iterator registryIterator = registrations_.begin();
    while (registryIterator != registrations_.end())
    {
        PhysicsObjectForceRegistration registry = *registryIterator;
        if (registry.physicsObject == physicsObject)
        {
            registrations_.erase(registryIterator);
            break;
        }

        ++registryIterator;
    }
}

void PhysicsObjectForceRegistry::Clear()
{
    registrations_.clear();
}

PhysicsObjectGravityForceGenerator::PhysicsObjectGravityForceGenerator(const Vector3& gravity) :
    gravity_(gravity)
{
}

void PhysicsObjectGravityForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    if (!physicsObject->HasFiniteMass())
    {
        return;
    }

    physicsObject->AddForce(gravity_ * physicsObject->GetMass());
}

void PhysicsObjectDragForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    Vector3 force;
    physicsObject->GetVelocity(&force);

    // Calculate the total drag coefficient
    float dragCoeff = force.Length();
    dragCoeff = k1_ * dragCoeff + k2 * dragCoeff * dragCoeff;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -dragCoeff;
    physicsObject->AddForce(force);
}

void PhysicsObjectSpringForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = physicsObject->GetWorldPosition();
    force -= other_->GetWorldPosition();

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    magnitude = std::abs(magnitude - restLength_);
    magnitude *= springConstant_;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    physicsObject->AddForce(force);
}

void PhysicsObjectBuoyancyForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Calculate the submersion depth
    float depth = physicsObject->GetWorldPosition().y;

    // Check if we're out of the water
    if (depth >= waterHeight_ + maxDepth_) return;
    Vector3 force(0,0,0);

    // Check if we're at maximum depth
    if (depth <= waterHeight_ - maxDepth_)
    {
        force.y = liquidDensity_ * volume_;
        physicsObject->AddForce(force);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity_ * volume_ * (depth - maxDepth_ - waterHeight_) / (2 * maxDepth_);
    physicsObject->AddForce(force);
}

void PhysicsObjectBungeeForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = physicsObject->GetWorldPosition();
    force -= other_->GetWorldPosition();

    // Check if the bungee is compressed
    float magnitude = force.Length();
    if (magnitude <= restLength_) return;

    // Calculate the magnitude of the force
    magnitude = springConstant_ * (restLength_ - magnitude);

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    physicsObject->AddForce(force);
}

void PhysicsObjectFakeSpringForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Check that we do not have infInite mass
    if (!physicsObject->HasFiniteMass()) return;

    // Calculate the relative position of the physicsObject to the anchor
    Vector3 position = physicsObject->GetWorldPosition();
    position -= *anchor_;

    // Calculate the constants and check they are in bounds.
    float gamma = 0.5f * std::sqrtf(4 * springConstant_ - damping_*damping_);
    if (gamma == 0.0f)
    {
        return;
    }

    Vector3 c = position * (damping_ / (2.0f * gamma)) + physicsObject->GetVelocity() * (1.0f / gamma);

    // Calculate the tarGet position
    Vector3 tarGet = position * std::cosf(gamma * duration) +
        c * std::sinf(gamma * duration);
    tarGet *= std::expf(-0.5f * duration * damping_);

    // Calculate the resulting acceleration and therefore the force
    Vector3 accel = (tarGet - position) * ((float)1.0 / (duration*duration)) - physicsObject->GetVelocity() * ((float)1.0/duration);
    physicsObject->AddForce(accel * physicsObject->GetMass());
}

void PhysicsObjectAnchoredSpringForceGenerator::Init(Vector3* anchor, float springConstant, float restLength)
{
    anchor_ = anchor;
    springConstant_ = springConstant;
    restLength_ = restLength;
}

void PhysicsObjectAnchoredBungeeForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = physicsObject->GetWorldPosition();
    force -= *anchor_;

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    if (magnitude < restLength_) return;

    magnitude = magnitude - restLength_;
    magnitude *= springConstant_;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    physicsObject->AddForce(force);
}

void PhysicsObjectAnchoredSpringForceGenerator::UpdateForce(PhysicsObject* physicsObject, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = physicsObject->GetWorldPosition();
    force -= *anchor_;

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    magnitude = (restLength_ - magnitude) * springConstant_;

    // Calculate the final force and apply it
    force.Normalize();
    force *= magnitude;
    physicsObject->AddForce(force);
}