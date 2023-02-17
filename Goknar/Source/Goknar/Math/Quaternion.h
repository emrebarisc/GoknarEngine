#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "Core.h"
#include "MathDefines.h"

class Matrix;
class Matrix3x3;
class Vector3;
class Vector4;

class GOKNAR_API Quaternion
{
public:
    Quaternion() : 
        x(0.f),
        y(0.f),
        z(0.f),
        w(1.f)
    {}

    Quaternion(float xValue, float yValue, float zValue, float wValue) :
        x(xValue),
        y(yValue),
        z(zValue),
        w(wValue)
    {}

    Quaternion(const Vector4& v);
    Quaternion(const Matrix3x3& rotationMatrix);
    Quaternion(float pitch/*rotation y*/, float yaw/*rotation z*/, float roll/*rotation x*/);
    Quaternion(Vector3 axis, float angle);
    Quaternion(const Vector3& normalized);

    inline Matrix3x3 GetMatrix() const;

    inline bool operator==(const Quaternion& other) const;
    inline bool operator!=(const Quaternion& other) const;

    inline Quaternion operator+(const Quaternion& other) const;
    inline Quaternion operator+=(const Quaternion& other);

    inline Quaternion operator-(const Quaternion& other) const;
    inline Quaternion operator-=(const Quaternion& other);

    inline Quaternion operator/(const float scale) const;
    inline Quaternion operator/=(const float scale);

    inline Quaternion operator*(const Matrix& scaleMatrix) const;
    inline Quaternion operator*(const Quaternion& other) const;
    inline Quaternion operator*(const float scale) const;
    inline Quaternion& operator*=(const Matrix& scaleMatrix);
    inline Quaternion& operator*=(const float scale);

    bool Equals(const Quaternion& other, float tolerance = EPSILON) const;

    Quaternion& Normalize();
    Quaternion& Conjugate();
    Vector3 Rotate(const Vector3& value) const;

    static const Quaternion Identity;

    float x, y, z, w;
};

#endif