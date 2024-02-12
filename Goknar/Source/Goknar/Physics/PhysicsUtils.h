#ifndef __PHYSICSUTILS_H__
#define __PHYSICSUTILS_H__

#include "Core.h"
#include "Math/GoknarMath.h"
#include "Math/Quaternion.h"

#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"

class GOKNAR_API PhysicsUtils
{
public:
    static inline btVector3 FromVector3ToBtVector3(const Vector3& vector)
    {
        return btVector3(vector.x, vector.y, vector.z);
    }

    static inline Vector3 FromBtVector3ToVector3(const btVector3& vector)
    {
        return Vector3(vector.x(), vector.y(), vector.z());
    }

    static inline btQuaternion FromQuaternionToBtQuaternion(const Quaternion& quaternion)
    {
        return btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
    }

    static inline Quaternion FromBtQuaternionToQuaternion(const btQuaternion& bulletQuaternion)
    {
        return Quaternion(bulletQuaternion.x(), bulletQuaternion.y(), bulletQuaternion.z(), bulletQuaternion.w());
    }
};

#endif