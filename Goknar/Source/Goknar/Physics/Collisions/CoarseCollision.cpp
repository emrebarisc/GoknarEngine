#include "pch.h"

#include "CoarseCollision.h"

BoundingSphere::BoundingSphere(const BoundingSphere& one, const BoundingSphere& two)
{
    Vector3 centerOffset = two.center_ - one.center_;
    float distance = centerOffset.SquareLength();
    float radiusDiff = two.radius_ - one.radius_;

    // Check if the larger sphere encloses the small one
    if (distance <= radiusDiff * radiusDiff)
    {
        if (one.radius_ > two.radius_)
        {
            center_ = one.center_;
            radius_ = one.radius_;
        }
        else
        {
            center_ = two.center_;
            radius_ = two.radius_;
        }
    }

    // Otherwise we need to work with partially
    // overlapping spheres
    else
    {
        distance = sqrtf(distance);
        radius_ = (distance + one.radius_ + two.radius_) * ((float)0.5);

        // The new center_ is based on one's center, moved towards
        // two's center_ by an ammount proportional to the spheres'
        // radii.
        center_ = one.center_;
        if (distance > 0)
        {
            center_ += centerOffset * ((radius_ - one.radius_)/distance);
        }
    }
}

bool BoundingSphere::Overlaps(const BoundingSphere *other) const
{
    float distanceSquared = (center_ - other->center_).SquareLength();
    return distanceSquared < (radius_ + other->radius_) * (radius_ + other->radius_);
}

float BoundingSphere::GetGrowth(const BoundingSphere &other) const
{
    BoundingSphere newSphere(*this, other);

    // We return a value proportional to the change in surface
    // area of the sphere.
    return newSphere.radius_ * newSphere.radius_ - radius_ * radius_;
}