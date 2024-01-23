#include "pch.h"

#include <cfloat>

#include "FineCollision.h"

#include "Goknar/GoknarAssert.h"
#include "Goknar/Physics/PhysicsConstants.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Contacts/PhysicsContact.h"

void CollisionData::AddContacts(unsigned int count)
{
    // Reduce the number of contacts remaining, add number used
    contactsLeft -= count;
    contactCount += count;

    // Move the array forward
    contacts += count;
}

void CollisionPrimitive::CalculateInternals()
{
}

Vector3 CollisionPrimitive::GetAxis(unsigned int index) const
{
    return body->GetWorldTransformationMatrixWithoutScaling().GetAxisVector(index);
}

const Matrix& CollisionPrimitive::GetTransform() const
{
    return body->GetWorldTransformationMatrixWithoutScaling();
}

bool IntersectionTests::SphereAndHalfSpace(const CollisionSphere& sphere, const CollisionPlane& plane)
{
    // Find the distance from the origin
    float ballDistance = plane.direction.Dot(sphere.GetAxis(3)) - sphere.radius;

    // Check for the intersection
    return ballDistance <= plane.offset;
}

bool IntersectionTests::SphereAndSphere(const CollisionSphere& one, const CollisionSphere& two)
{
    // Find the vector between the objects
    Vector3 midline = one.GetAxis(3) - two.GetAxis(3);

    // See if it is large enough.
    return midline.SquareLength() < (one.radius + two.radius) * (one.radius + two.radius);
}

static inline float transformToAxis(const CollisionBox& box, const Vector3& axis)
{
    return  box.halfSize.x * std::abs(axis.Dot(box.GetAxis(0))) +
            box.halfSize.y * std::abs(axis.Dot(box.GetAxis(1))) +
            box.halfSize.z * std::abs(axis.Dot(box.GetAxis(2)));
}

/**
 * This function checks if the two boxes overlap
 * along the given axis. The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline bool OverlapOnAxis(const CollisionBox& one, const CollisionBox& two, const Vector3& axis, const Vector3& toCentre)
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = std::abs(toCentre.Dot(axis));

    // Check for overlap
    return (distance < oneProject + twoProject);
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox intersection  method.
#define TEST_OVERLAP(axis) OverlapOnAxis(one, two, (axis), toCentre)

bool IntersectionTests::BoxAndBox(const CollisionBox& one, const CollisionBox& two)
{
    // Find the vector between the two centres
    Vector3 toCentre = two.GetAxis(3) - one.GetAxis(3);

    return (
        // Check on box one's axes first
        TEST_OVERLAP(one.GetAxis(0)) &&
        TEST_OVERLAP(one.GetAxis(1)) &&
        TEST_OVERLAP(one.GetAxis(2)) &&

        // And on two's
        TEST_OVERLAP(two.GetAxis(0)) &&
        TEST_OVERLAP(two.GetAxis(1)) &&
        TEST_OVERLAP(two.GetAxis(2)) &&

        // Now on the cross products
        TEST_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(0))) &&
        TEST_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(1))) &&
        TEST_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(2))) &&
        TEST_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(0))) &&
        TEST_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(1))) &&
        TEST_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(2))) &&
        TEST_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(0))) &&
        TEST_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(1))) &&
        TEST_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(2))));
}
#undef TEST_OVERLAP

bool IntersectionTests::BoxAndHalfSpace(const CollisionBox& box, const CollisionPlane& plane)
{
    // Work out the projected radius of the box onto the plane direction
    float projectedRadius = transformToAxis(box, plane.direction);

    // Work out how far the box is from the origin
    float boxDistance = plane.direction.Dot(box.GetAxis(3)) - projectedRadius;

    // Check for the intersection
    return boxDistance <= plane.offset;
}

unsigned int CollisionDetector::SphereAndTruePlane(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionData* data)
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Cache the sphere position
    Vector3 position = sphere.GetAxis(3);

    // Find the distance from the plane
    float centreDistance = plane.direction.Dot(position) - plane.offset;

    // Check if we're within radius
    if (centreDistance * centreDistance > sphere.radius * sphere.radius)
    {
        return 0;
    }

    // Check which side of the plane we're on
    Vector3 normal = plane.direction;
    float penetration = -centreDistance;
    if (centreDistance < 0)
    {
        normal *= -1;
        penetration = -penetration;
    }
    penetration += sphere.radius;

    // Create the contact - it has a normal in the plane direction.
    PhysicsContact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->penetration = penetration;
    contact->contactPoint = position - plane.direction * centreDistance;
    contact->SetBodyData(sphere.body, nullptr, data->friction, data->restitution);

    data->AddContacts(1);
    return 1;
}

unsigned int CollisionDetector::SphereAndHalfSpace(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionData* data)
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0)
    {
        return 0;
    }

    // Cache the sphere position
    Vector3 position = sphere.GetAxis(3);

    // Find the distance from the plane
    float ballDistance = plane.direction.Dot(position) - sphere.radius - plane.offset;

    if (ballDistance >= 0)
    {
        return 0;
    }

    // Create the contact - it has a normal in the plane direction.
    PhysicsContact* contact = data->contacts;
    contact->contactNormal = plane.direction;
    contact->penetration = -ballDistance;
    contact->contactPoint = position - plane.direction * (ballDistance + sphere.radius);
    contact->SetBodyData(sphere.body, nullptr, data->friction, data->restitution);

    data->AddContacts(1);
    return 1;
}

unsigned int CollisionDetector::SphereAndSphere(const CollisionSphere& one, const CollisionSphere& two, CollisionData* data)
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0)
    {
        return 0;
    }

    // Cache the sphere positions
    Vector3 positionOne = one.GetAxis(3);
    Vector3 positionTwo = two.GetAxis(3);

    // Find the vector between the objects
    Vector3 midline = positionOne - positionTwo;
    float size = midline.Length();

    // See if it is large enough.
    if (size <= 0.0f || size >= one.radius + two.radius)
    {
        return 0;
    }

    // We manually create the normal, because we have the
    // size to hand.
    Vector3 normal = midline * (((float)1.0) / size);

    PhysicsContact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->contactPoint = positionOne + midline * (float)0.5;
    contact->penetration = (one.radius + two.radius - size);
    contact->SetBodyData(one.body, two.body, data->friction, data->restitution);

    data->AddContacts(1);
    return 1;
}

/*
 * This function checks if the two boxes overlap
 * along the given axis, returning the ammount of overlap.
 * The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline float PenetrationOnAxis(const CollisionBox& one, const CollisionBox& two, const Vector3& axis, const Vector3& toCentre)
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = std::abs(toCentre.Dot(axis));

    // Return the overlap (i.e. positive indicates
    // overlap, negative indicates separation).
    return oneProject + twoProject - distance;
}


static inline bool TryAxis(const CollisionBox& one, const CollisionBox& two, Vector3 axis, const Vector3& toCentre, unsigned int index, float& smallestPenetration, unsigned int& smallestCase)
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (axis.SquareLength() < 0.0001f)
    {
        return true;
    }
    axis.Normalize();

    float penetration = PenetrationOnAxis(one, two, axis, toCentre);

    if (penetration < 0.f)
    {
        return false;
    }

    if (penetration < smallestPenetration)
    {
        smallestPenetration = penetration;
        smallestCase = index;
    }

    return true;
}

void FillPointFaceBoxBox(const CollisionBox& one, const CollisionBox& two, const Vector3& toCentre, CollisionData* data, unsigned int best, float pen)
{
    // This method is called when we know that a vertex from
    // box two is in contact with box one.

    PhysicsContact* contact = data->contacts;

    // We know which axis the collision is on (i.e. best),
    // but we need to work out which of the two faces on
    // this axis.
    Vector3 normal = one.GetAxis(best);
    if (0 < one.GetAxis(best).Dot(toCentre))
    {
        normal *= -1.f;
    }

    // Work out which vertex of box two we're colliding with.
    // Using toCentre doesn't work!
    Vector3 vertex = two.halfSize;
    if (two.GetAxis(0).Dot(normal) < 0)
    {
        vertex.x = -vertex.x;
    }

    if (two.GetAxis(1).Dot(normal) < 0)
    {
        vertex.y = -vertex.y;
    }

    if (two.GetAxis(2).Dot(normal) < 0)
    {
        vertex.z = -vertex.z;
    }

    // Create the contact data
    contact->contactNormal = normal;
    contact->penetration = pen;
    contact->contactPoint = two.GetTransform() * Vector4(vertex, 1.f);
    contact->SetBodyData(one.body, two.body, data->friction, data->restitution);
}

static inline Vector3 ContactPoint(const Vector3& pOne, const Vector3& dOne, float oneSize, const Vector3& pTwo, const Vector3& dTwo, float twoSize,
    // If this is true, and the contact point is outside
    // the edge (in the case of an edge-face contact) then
    // we use one's midpoint, otherwise we use two's.
    bool useOne)
{
    Vector3 toSt, cOne, cTwo;
    float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
    float denom, mua, mub;

    smOne = dOne.SquareLength();
    smTwo = dTwo.SquareLength();
    dpOneTwo = dTwo.Dot(dOne);

    toSt = pOne - pTwo;
    dpStaOne = dOne.Dot(toSt);
    dpStaTwo = dTwo.Dot(toSt);

    denom = smOne * smTwo - dpOneTwo * dpOneTwo;

    // Zero denominator indicates parrallel lines
    if (std::abs(denom) < 0.0001f) {
        return useOne ? pOne : pTwo;
    }

    mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
    mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

    // If either of the edges has the nearest point out
    // of bounds, then the edges aren't crossed, we have
    // an edge-face contact. Our point is on the edge, which
    // we know from the useOne parameter.
    if (mua > oneSize ||
        mua < -oneSize ||
        mub > twoSize ||
        mub < -twoSize)
    {
        return useOne ? pOne : pTwo;
    }
    else
    {
        cOne = pOne + dOne * mua;
        cTwo = pTwo + dTwo * mub;

        return cOne * 0.5 + cTwo * 0.5;
    }
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!TryAxis(one, two, (axis), toCentre, (index), pen, best)) return 0;

unsigned int CollisionDetector::BoxAndBox(const CollisionBox& one, const CollisionBox& two, CollisionData* data)
{
    //if (!IntersectionTests::BoxAndBox(one, two)) return 0;

    // Find the vector between the two centres
    Vector3 toCentre = two.body->GetWorldPosition() - one.body->GetWorldPosition();

    // We start assuming there is no contact
    float pen = FLT_MAX;
    unsigned int best = 0xffffff;

    // Now we check each axes, returning if it gives us
    // a separating axis, and keeping track of the axis with
    // the smallest penetration otherwise.
    CHECK_OVERLAP(one.GetAxis(0), 0);
    CHECK_OVERLAP(one.GetAxis(1), 1);
    CHECK_OVERLAP(one.GetAxis(2), 2);

    CHECK_OVERLAP(two.GetAxis(0), 3);
    CHECK_OVERLAP(two.GetAxis(1), 4);
    CHECK_OVERLAP(two.GetAxis(2), 5);

    // Store the best axis-major, in case we run into almost
    // parallel edge collisions later
    unsigned int bestSingleAxis = best;

    CHECK_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(0)), 6);
    CHECK_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(1)), 7);
    CHECK_OVERLAP(one.GetAxis(0).Cross(two.GetAxis(2)), 8);
    CHECK_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(0)), 9);
    CHECK_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(1)), 10);
    CHECK_OVERLAP(one.GetAxis(1).Cross(two.GetAxis(2)), 11);
    CHECK_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(0)), 12);
    CHECK_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(1)), 13);
    CHECK_OVERLAP(one.GetAxis(2).Cross(two.GetAxis(2)), 14);

    // Make sure we've got a result.
    GOKNAR_ASSERT(best != 0xffffff);

    // We now know there's a collision, and we know which
    // of the axes gave the smallest penetration. We now
    // can deal with it in different ways depending on
    // the case.
    if (best < 3)
    {
        // We've got a vertex of box two on a face of box one.
        FillPointFaceBoxBox(one, two, toCentre, data, best, pen);
        data->contacts->SetBodyData(one.body, two.body, data->friction, data->restitution);
        data->AddContacts(1);
        return 1;
    }
    else if (best < 6)
    {
        // We've got a vertex of box one on a face of box two.
        // We use the same algorithm as above, but swap around
        // one and two (and therefore also the vector between their
        // centres).
        FillPointFaceBoxBox(two, one, toCentre * -1.0f, data, best - 3, pen);
        data->contacts->SetBodyData(one.body, two.body, data->friction, data->restitution);
        data->AddContacts(1);
        return 1;
    }
    else
    {
        // We've got an edge-edge contact. Find out which axes
        best -= 6;
        unsigned int oneAxisIndex = best / 3;
        unsigned int twoAxisIndex = best % 3;
        Vector3 oneAxis = one.GetAxis(oneAxisIndex);
        Vector3 twoAxis = two.GetAxis(twoAxisIndex);
        Vector3 axis = oneAxis.Cross(twoAxis);
        axis.Normalize();

        // The axis should point from box one to box two.
        if (0.f < axis.Dot(toCentre))
        {
            axis = axis * -1.f;
        }

        // We have the axes, but not the edges: each axis has 4 edges parallel
        // to it, we need to find which of the 4 for each object. We do
        // that by finding the point in the centre of the edge. We know
        // its component in the direction of the box's collision axis is zero
        // (its a mid-point) and we determine which of the extremes in each
        // of the other axes is closest.
        Vector3 ptOnOneEdge = one.halfSize;
        Vector3 ptOnTwoEdge = two.halfSize;
        for (unsigned int i = 0; i < 3; i++)
        {
            if (i == oneAxisIndex)
            {
                ptOnOneEdge[i] = 0;
            }
            else if (0.f < one.GetAxis(i).Dot(axis))
            {
                ptOnOneEdge[i] = -ptOnOneEdge[i];
            }

            if (i == twoAxisIndex)
            {
                ptOnTwoEdge[i] = 0;
            }
            else if (two.GetAxis(i).Dot(axis) < 0.f)
            {
                ptOnTwoEdge[i] = -ptOnTwoEdge[i];
            }
        }

        // Move them into world coordinates (they are already oriented
        // correctly, since they have been derived from the axes).
        ptOnOneEdge = one.body->GetWorldTransformationMatrix() * Vector4(ptOnOneEdge, 1.f);
        ptOnTwoEdge = two.body->GetWorldTransformationMatrix() * Vector4(ptOnTwoEdge, 1.f);

        // So we have a point and a direction for the colliding edges.
        // We need to find out point of closest approach of the two
        // line-segments.
        Vector3 vertex = ContactPoint(ptOnOneEdge, oneAxis, one.halfSize[oneAxisIndex], ptOnTwoEdge, twoAxis, two.halfSize[twoAxisIndex], 2 < bestSingleAxis);

        // We can fill the contact.
        PhysicsContact* contact = data->contacts;

        contact->penetration = pen;
        contact->contactNormal = axis;
        contact->contactPoint = vertex;
        contact->SetBodyData(one.body, two.body, data->friction, data->restitution);
        data->AddContacts(1);
        return 1;
    }
    return 0;
}
#undef CHECK_OVERLAP




unsigned int CollisionDetector::BoxAndPoint(const CollisionBox& box, const Vector3& point, CollisionData* data)
{
    // Transform the point into box coordinates
    Vector3 relPt = box.body->GetWorldTransformationMatrix().MultiplyTransposeByInverse(point);

    Vector3 normal;

    // Check each axis, looking for the axis on which the
    // penetration is least deep.
    float min_depth = box.halfSize.x - std::abs(relPt.x);
    if (min_depth < 0)
    {
        return 0;
    }

    normal = box.GetAxis(0) * ((relPt.x < 0.f) ? -1.f : 1.f);

    float depth = box.halfSize.y - std::abs(relPt.y);
    if (depth < 0)
    {
        return 0;
    }
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.GetAxis(1) * ((relPt.y < 0.f) ? -1.f : 1.f);
    }

    depth = box.halfSize.z - std::abs(relPt.z);
    if (depth < 0)
    {
        return 0;
    }
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.GetAxis(2) * ((relPt.z < 0.f) ? -1.f : 1.f);
    }

    // Compile the contact
    PhysicsContact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->contactPoint = point;
    contact->penetration = min_depth;

    // Note that we don't know what rigid body the point
    // belongs to, so we just use nullptr. Where this is called
    // this value can be left, or filled in.
    contact->SetBodyData(box.body, nullptr, data->friction, data->restitution);

    data->AddContacts(1);
    return 1;
}

unsigned int CollisionDetector::BoxAndSphere(const CollisionBox& box, const CollisionSphere& sphere, CollisionData* data)
{
    // Transform the centre of the sphere into box coordinates
    Vector3 sphereWorldPosition = sphere.body->GetWorldPosition();
    Vector3 relativeSphereWorldPosition = box.body->GetWorldRotation().GetInverse().GetMatrix() * Vector4(sphereWorldPosition - box.body->GetWorldPosition(), 1.f);

    // Early out check to see if we can exclude the contact
    if (std::abs(relativeSphereWorldPosition.x) - sphere.radius > box.halfSize.x ||
        std::abs(relativeSphereWorldPosition.y) - sphere.radius > box.halfSize.y ||
        std::abs(relativeSphereWorldPosition.z) - sphere.radius > box.halfSize.z)
    {
        return 0;
    }

    Vector3 relativeContactPosition = Vector3::ZeroVector;
    relativeContactPosition.x = GoknarMath::Clamp(relativeSphereWorldPosition.x, -box.halfSize.x, box.halfSize.x);
    relativeContactPosition.y = GoknarMath::Clamp(relativeSphereWorldPosition.y, -box.halfSize.y, box.halfSize.y);
    relativeContactPosition.z = GoknarMath::Clamp(relativeSphereWorldPosition.z, -box.halfSize.z, box.halfSize.z);

    // If the sphere is inside the box, reference the closest box face
    if ((relativeContactPosition - relativeSphereWorldPosition).SquareLength() < PHYSICS_COLLISION_EPSILON)
    {
        bool isSphereInsideTheBoxOnX = -box.halfSize.x < relativeContactPosition.x || relativeContactPosition.x < box.halfSize.x;
        bool isSphereInsideTheBoxOnY = -box.halfSize.y < relativeContactPosition.y || relativeContactPosition.y < box.halfSize.y;
        bool isSphereInsideTheBoxOnZ = -box.halfSize.z < relativeContactPosition.z || relativeContactPosition.z < box.halfSize.z;

        if (isSphereInsideTheBoxOnX && isSphereInsideTheBoxOnY && isSphereInsideTheBoxOnZ)
        {
            float xDistanceToFace = GoknarMath::Abs(relativeContactPosition.x < 0.f ? relativeContactPosition.x + box.halfSize.x : relativeContactPosition.x - box.halfSize.x);
            float yDistanceToFace = GoknarMath::Abs(relativeContactPosition.y < 0.f ? relativeContactPosition.y + box.halfSize.y : relativeContactPosition.y - box.halfSize.y);
            float zDistanceToFace = GoknarMath::Abs(relativeContactPosition.z < 0.f ? relativeContactPosition.z + box.halfSize.z : relativeContactPosition.z - box.halfSize.z);

            if (xDistanceToFace < yDistanceToFace && xDistanceToFace < zDistanceToFace)
            {
                relativeContactPosition.x = relativeContactPosition.x < 0.f ? -box.halfSize.x : box.halfSize.x;
            }
            else if (yDistanceToFace < xDistanceToFace && yDistanceToFace < zDistanceToFace)
            {
                relativeContactPosition.y = relativeContactPosition.y < 0.f ? -box.halfSize.y : box.halfSize.y;
            }
            else
            {
                relativeContactPosition.z = relativeContactPosition.z < 0.f ? -box.halfSize.z : box.halfSize.z;
            }
        }
    }

    // Check we're in contact
    float squareDistance = (relativeContactPosition - relativeSphereWorldPosition).SquareLength();
    if (sphere.radius * sphere.radius < squareDistance)
    {
        return 0;
    }

    // Compile the contact
    Vector3 contactPosition = box.body->GetWorldPosition() + box.body->GetWorldRotation().GetMatrix() * Vector4(relativeContactPosition, 1.f);
    Vector3 normal = contactPosition - sphereWorldPosition;

    PhysicsContact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->contactPoint = contactPosition;
    contact->penetration = sphere.radius - sqrtf(squareDistance);
    contact->SetBodyData(box.body, sphere.body, data->friction, data->restitution);

    data->AddContacts(1);
    return 1;
}

unsigned int CollisionDetector::BoxAndHalfSpace(const CollisionBox& box, const CollisionPlane& plane, CollisionData* data)
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Check for intersection
    if (!IntersectionTests::BoxAndHalfSpace(box, plane))
    {
        return 0;
    }

    // We have an intersection, so find the intersection points. We can make
    // do with only checking vertices. If the box is resting on a plane
    // or on an edge, it will be reported as four or two contact points.

    // Go through each combination of + and - for each half-size
    static float mults[8][3] = { {1,1,1},{-1,1,1},{1,-1,1},{-1,-1,1},
                               {1,1,-1},{-1,1,-1},{1,-1,-1},{-1,-1,-1} };

    PhysicsContact* contact = data->contacts;
    unsigned int contactsUsed = 0;
    for (unsigned int i = 0; i < 8; i++) {

        // Calculate the position of each vertex
        Vector3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);
        vertexPos *= box.halfSize;
        vertexPos = box.body->GetWorldTransformationMatrix() * Vector4(vertexPos, 1.f);

        // Calculate the distance from the plane
        float vertexDistance = vertexPos.Dot(plane.direction);

        // Compare this to the plane's distance
        if (vertexDistance <= plane.offset)
        {
            // Create the contact data.

            // The contact point is halfway between the vertex and the
            // plane - we multiply the direction by half the separation
            // distance and add the vertex location.
            contact->contactPoint = plane.direction;
            contact->contactPoint *= (vertexDistance - plane.offset);
            contact->contactPoint += vertexPos;
            contact->contactNormal = plane.direction;
            contact->penetration = plane.offset - vertexDistance;

            // Write the appropriate data
            contact->SetBodyData(box.body, nullptr, data->friction, data->restitution);

            // Move onto the next contact
            contact++;
            contactsUsed++;
            if (contactsUsed == (unsigned int)data->contactsLeft) return contactsUsed;
        }
    }

    data->AddContacts(contactsUsed);
    return contactsUsed;
}
