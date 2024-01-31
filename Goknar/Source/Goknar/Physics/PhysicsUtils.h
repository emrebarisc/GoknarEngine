#ifndef __PHYSICSUTILS_H__
#define __PHYSICSUTILS_H__

#include "Core.h"
#include "Math/GoknarMath.h"

#include "LinearMath/btVector3.h"

class GOKNAR_API PhysicsUtils
{
public:
    static btVector3 FromVector3ToBtVector3(const Vector3& vector)
    {
        return btVector3(vector.x, vector.y, vector.z);
    }

    static Vector3 FromBtVector3ToVector3(const btVector3& vector)
    {
        return Vector3(vector.x(), vector.y(), vector.z());
    }
};

#endif