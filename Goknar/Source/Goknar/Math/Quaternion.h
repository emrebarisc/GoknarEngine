#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "Core.h"
#include "MathDefines.h"

#include <cmath>

class Matrix;
class Matrix3x3;
class Vector3;
class Vector4;

//  References: 
//      https://users.aalto.fi/~ssarkka/pub/quat.pdf

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

    Quaternion(const Matrix& rotationMatrix);
    Quaternion(const Matrix3x3& rotationMatrix);
    Quaternion(float pitch/*rotation y*/, float yaw/*rotation z*/, float roll/*rotation x*/);

    Quaternion(Vector3 axis, float angle);

    static Quaternion FromEular(const Vector3& degrees);
    static Quaternion FromEularRadians(const Vector3& radians);

    inline Matrix GetMatrix() const;
    inline Matrix3x3 GetMatrix3x3() const;

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
    inline Quaternion operator*(const float scale);
    inline friend Quaternion operator*(const float scale, const Quaternion& quaternion);
    inline Quaternion& operator*=(const Matrix& scaleMatrix);
    inline Quaternion& operator*=(const float scale);

    static void Slerp(Quaternion& out, const Quaternion& start, const Quaternion& end, float alpha);

    bool Equals(const Quaternion& other, float tolerance = EPSILON) const;

    // NOT TESTED !
    Quaternion Pow(float n);
    // NOT TESTED !
    Quaternion Exp();
    // NOT TESTED !
    Quaternion Ln();

    inline float Length() const;

    inline Quaternion GetNormalized() const;
    inline Quaternion& Normalize();

    inline Quaternion GetConjugate() const;
    inline Quaternion& Conjugate();

    inline Quaternion GetInverse() const;
    inline Quaternion& Invert();

    Vector3 Rotate(const Vector3& value) const;

    static const Quaternion Identity;

    float x, y, z, w;
};

inline bool Quaternion::operator==(const Quaternion& other) const
{
    return  x == other.x &&
        y == other.y &&
        z == other.z &&
        w == other.w;
}

inline bool Quaternion::operator!=(const Quaternion& other) const
{
    return !(*this == other);
}

inline Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

inline Quaternion Quaternion::operator+=(const Quaternion& other)
{
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
    this->w += other.w;

    return *this;
}

inline Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

inline Quaternion Quaternion::operator-=(const Quaternion& other)
{
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
    this->w -= other.w;

    return *this;
}

inline Quaternion Quaternion::operator/(const float scale) const
{
    const float inverseScale = 1.f / scale;
    return Quaternion(x * inverseScale, y * inverseScale, z * inverseScale, w * inverseScale);
}

inline Quaternion Quaternion::operator/=(const float scale)
{
    const float inverseScale = 1.f / scale;
    x *= inverseScale;
    y *= inverseScale;
    z *= inverseScale;
    w *= inverseScale;

    return *this;
}


inline Quaternion operator*(const float scale, const Quaternion& quaternion)
{
    return Quaternion(quaternion.x * scale, quaternion.y * scale, quaternion.z * scale, quaternion.w * scale);
}

inline Quaternion Quaternion::operator*(const Quaternion& t) const
{
    return Quaternion(  w * t.w - x * t.x - y * t.y - z * t.z,
                        w * t.x + x * t.w + y * t.z - z * t.y,
                        w * t.y + y * t.w + z * t.x - x * t.z,
                        w * t.z + z * t.w + x * t.y - y * t.x);
}

inline Quaternion Quaternion::operator*(const float scale)
{
    return Quaternion(x * scale, y * scale, z * scale, w * scale);
}

inline Quaternion& Quaternion::operator*=(const float scale)
{
    x *= scale;
    y *= scale;
    z *= scale;
    w *= scale;

    return *this;
}

float Quaternion::Length() const
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

inline Quaternion Quaternion::GetNormalized() const
{
    Quaternion q(*this);
    q.Normalize();
    return q;
}

inline Quaternion& Quaternion::Normalize()
{
    const float magnitude = std::sqrt(x * x + y * y + z * z + w * w);
    if (magnitude)
    {
        const float inverseMagnitude = 1.f / magnitude;
        x *= inverseMagnitude;
        y *= inverseMagnitude;
        z *= inverseMagnitude;
        w *= inverseMagnitude;
    }
    return *this;
}

inline Quaternion Quaternion::GetConjugate() const
{
    Quaternion q(*this);
    q.Conjugate();
    return q;
}

inline Quaternion& Quaternion::Conjugate()
{
    x = -x;
    y = -y;
    z = -z;
    return *this;
}

inline Quaternion Quaternion::GetInverse() const
{
    Quaternion q(*this);
    q.Invert();
    return q;
}

inline Quaternion& Quaternion::Invert()
{
    const float length = Length();
    const float squareLength = length * length;

    x = -x / squareLength;
    y = -y / squareLength;
    z = -z / squareLength;
    w = w / squareLength;

    return *this;
}

#endif