#include "pch.h"

#include "Goknar/Physics/ForceGenerators/ForceGenerator.h"

#include "Goknar/Physics/RigidBody.h"

ForceGenerator::ForceGenerator()
{

}

ForceRegistry::~ForceRegistry()
{
    Registry::iterator registrationIterator = registrations_.begin();
    while (registrationIterator != registrations_.end())
    {
        delete registrationIterator->forceGenerator;

        ++registrationIterator;
    }
}

void ForceRegistry::Add(RigidBody* rigidBody, ForceGenerator* forceGenerator)
{
    ForceRegistry::ForceRegistration registration;
    registration.rigidBody = rigidBody;
    registration.forceGenerator = forceGenerator;
    registrations_.push_back(registration);
}

void ForceRegistry::Remove(RigidBody* rigidBody, ForceGenerator* forceGenerator)
{
    Registry::iterator registryIterator = registrations_.begin();
    while (registryIterator != registrations_.end())
    {
        ForceRegistration registry = *registryIterator;
        if (registry.rigidBody == rigidBody)
        {
            registrations_.erase(registryIterator);
            break;
        }

        ++registryIterator;
    }
}

void ForceRegistry::Clear()
{
    registrations_.clear();
}

void ForceRegistry::UpdateForces(float duration)
{
    Registry::iterator registryIterator = registrations_.begin();
    for (; registryIterator != registrations_.end(); registryIterator++)
    {
        registryIterator->forceGenerator->UpdateForce(registryIterator->rigidBody, duration);
    }
}

void Gravity::UpdateForce(RigidBody* rigidBody, float duration)
{
    if (!rigidBody->HasFiniteMass())
    {
        return;
    }

    rigidBody->AddForce(gravity_ * rigidBody->GetMass());
}

void Spring::UpdateForce(RigidBody* rigidBody, float duration)
{
    // Calculate the two ends in world space
    Vector3 lws = rigidBody->GetRelativePositionInWorldSpace(connectionPoint_);
    Vector3 ows = other_->GetRelativePositionInWorldSpace(otherConnectionPoint_);

    // Calculate the vector of the spring
    Vector3 force = lws - ows;

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    magnitude = std::abs(magnitude - restLength_);
    magnitude *= springConstant_;

    // Calculate the final force and apply it
    force.Normalize ();
    force *= -magnitude;
    rigidBody->AddForceAtPoint(force, lws);
}

void Aero::UpdateForce(RigidBody* rigidBody, float deltaTime)
{
    UpdateForceFromTensor(rigidBody, deltaTime, tensor_);
}

void Aero::UpdateForceFromTensor(RigidBody* rigidBody, float deltaTime, const Matrix3x3& tensor)
{
    // Calculate total velocity (windspeed and body's velocity).
    Vector3 velocity = rigidBody->GetVelocity();
    velocity += *windSpeed_;

    // Calculate the velocity in body coordinates
    Vector3 bodyVel = rigidBody->GetWorldDirectionInRelativeSpace(velocity);

    // Calculate the force in body coordinates
    Vector3 bodyForce = tensor * bodyVel;
    Vector3 force = rigidBody->GetRelativeDirectionInWorldSpace(bodyForce);

    rigidBody->AddForceAtBodyPoint(force, position_);
}

void AeroControl::UpdateForce(RigidBody* rigidBody, float deltaTime)
{
    UpdateForceFromTensor(rigidBody, deltaTime, GetTensor());
}

Matrix3x3 AeroControl::GetTensor()
{
    if (controlSetting_ <= -1.f) return minTensor_;
    else if (1.f <= controlSetting_) return maxTensor_;
    else if (controlSetting_ < 0.f)
    {
        return Matrix3x3::Lerp(minTensor_, tensor_, controlSetting_ + 1.f);
    }
    else if (0.f < controlSetting_)
    {
        return Matrix3x3::Lerp(tensor_, maxTensor_, controlSetting_);
    }
    else return tensor_;
}

void AngledAero::UpdateForce(RigidBody* rigidBody, float duration)
{
    // TODO: Implement
}

void Buoyancy::UpdateForce(RigidBody* rigidBody, float duration)
{
    // Calculate the submersion depth
    float depth = rigidBody->GetWorldPosition().z;

    // Check if we're out of the water
    if (depth >= waterHeight_ + maxDepth_)
    {
        return;
    }
    Vector3 force = Vector3::ZeroVector;

    // Check if we're at maximum depth
    if (depth <= waterHeight_ - maxDepth_)
    {
        force.z = liquidDensity_ * volume_;
        rigidBody->AddForce(force);
        return;
    }

    // Otherwise we are partly submerged
    force.z = liquidDensity_ * volume_ * (depth - maxDepth_ - waterHeight_) / (2 * maxDepth_);
    rigidBody->AddForce(force);
}

void DragForceGenerator::UpdateForce(RigidBody* rigidBody, float deltaTime)
{
    Vector3 force;
    rigidBody->GetVelocity(&force);

    // Calculate the total drag coefficient
    float dragCoeff = force.Length();
    dragCoeff = k1_ * dragCoeff + k2_ * dragCoeff * dragCoeff;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -dragCoeff;
    rigidBody->AddForce(force);
}

void AnchoredSpringForceGenerator::UpdateForce(RigidBody* rigidBody, float duration)
{
    // Calculate the two ends in world space
    Vector3 lws = rigidBody->GetRelativePositionInWorldSpace(connectionPoint_);

    // Calculate the vector of the spring
    Vector3 force = lws - *anchor_;

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    magnitude = std::abs(magnitude - restLength_);
    magnitude *= springConstant_;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    rigidBody->AddForceAtPoint(force, lws);
}

void BungeeForceGenerator::UpdateForce(RigidBody* rigidBody, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = rigidBody->GetWorldPosition();
    force -= other_->GetWorldPosition();

    // Check if the bungee is compressed
    float magnitude = force.Length();
    if (magnitude <= restLength_) return;

    // Calculate the magnitude of the force
    magnitude = springConstant_ * (magnitude - restLength_);

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    rigidBody->AddForce(force);
}

void AnchoredBungeeForceGenerator::UpdateForce(RigidBody* rigidBody, float duration)
{
    // Calculate the vector of the spring
    Vector3 force = rigidBody->GetWorldPosition();
    force -= *anchor_;

    // Calculate the magnitude of the force
    float magnitude = force.Length();
    if (magnitude < restLength_) return;

    magnitude = magnitude - restLength_;
    magnitude *= springConstant_;

    // Calculate the final force and apply it
    force.Normalize();
    force *= -magnitude;
    rigidBody->AddForce(force);
}