#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "Core.h"
#include "MathDefines.h"

#include <cmath>

#include "Math/GoknarMath.h"

class Matrix;
class Matrix3x3;

//  References:
//      https://users.aalto.fi/~ssarkka/pub/quat.pdf
//      https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

// x, y, z are complex components and w is real component
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
    Quaternion(float roll/*rotation x*/, float pitch/*rotation y*/, float yaw/*rotation z*/);

    Quaternion(Vector3 axis, float angle);

    inline bool ContainsNanOrInf() const
    {
        return  GoknarMath::IsNanOrInf(x) ||
                GoknarMath::IsNanOrInf(y) ||
                GoknarMath::IsNanOrInf(z) ||
                GoknarMath::IsNanOrInf(w);
    }

    static Quaternion FromEulerDegrees(float x, float y, float z);
    static Quaternion FromEulerDegrees(const Vector3& degrees);
    static Quaternion FromEulerRadians(float x, float y, float z);
    static Quaternion FromEulerRadians(const Vector3& radians);

    static Quaternion FromTwoVectors(const Vector3& first, const Vector3& second);

    void AddVector(const Vector3& vector);

    Vector3 ToEulerDegrees() const;
    Vector3 ToEulerRadians() const;

    Matrix GetMatrix() const;
    Matrix3x3 GetMatrix3x3() const;

    inline Vector3 GetForwardVector() const;
    inline Vector3 GetUpVector() const;
    inline Vector3 GetLeftVector() const;

    inline bool operator==(const Quaternion& other) const;
    inline bool operator!=(const Quaternion& other) const;

    inline Quaternion operator+(const Quaternion& other) const;
    inline Quaternion& operator+=(const Quaternion& other);

    inline Quaternion operator-(const Quaternion& other) const;
    inline Quaternion operator-=(const Quaternion& other);

    inline Quaternion operator/(const float scale) const;
    inline Quaternion operator/=(const float scale);

    Quaternion operator*(const Matrix& scaleMatrix) const;
    Quaternion& operator*=(const Matrix& scaleMatrix);
    
    inline Quaternion operator*(const Quaternion& other) const;
    inline Quaternion operator*(const float scale);
    inline friend Quaternion operator*(const float scale, const Quaternion& quaternion);
    inline Quaternion& operator*=(const Quaternion& other);
    inline Quaternion& operator*=(const float scale);

    inline Vector3 operator*(const Vector3& v) const;

    inline float Dot(const Quaternion& rhs) const;

    static inline float GetAngleBetween(const Quaternion& first, const Quaternion& second);

    static inline Quaternion FromAxisAngle(const Vector3& axis, float angleRadians);

    static Quaternion Slerp(const Quaternion& start, const Quaternion& end, float alpha);

    bool Equals(const Quaternion& other, float tolerance = EPSILON) const;

    // TODO: TEST !
    Quaternion Pow(float n);
    // TODO: TEST !
    Quaternion Exp();
    // TODO: TEST !
    Quaternion Ln();

    inline float Length() const;

    inline Quaternion GetNormalized() const;
    inline Quaternion& Normalize();

    inline Quaternion GetConjugate() const;
    inline Quaternion& Conjugate();

    inline Quaternion GetInverse() const;
    inline Quaternion& Invert();

    Vector3 Rotate(const Vector3& value) const;

    inline std::string ToString() const
    {
        return std::to_string(w) + ", < " + std::to_string(x) + "i, " + std::to_string(y) + "j, " + std::to_string(z) + "k >";
    }

    static const Quaternion Identity;

    float x, y, z, w;
};

inline Vector3 Quaternion::GetForwardVector() const
{
    return Rotate(Vector3::ForwardVector);
}

inline Vector3 Quaternion::GetUpVector() const
{
    return Rotate(Vector3::UpVector);
}

inline Vector3 Quaternion::GetLeftVector() const
{
    return Rotate(Vector3::LeftVector);
}

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

inline Quaternion& Quaternion::operator+=(const Quaternion& other)
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

inline Quaternion Quaternion::operator*(const Quaternion& other) const
{
    return Quaternion(  x * other.w + w * other.x - z * other.y + y * other.z,
                        y * other.w + z * other.x + w * other.y - x * other.z,
                        z * other.w - y * other.x + x * other.y + w * other.z,
                        w * other.w - x * other.x - y * other.y - z * other.z);
}

inline Quaternion& Quaternion::operator*=(const Quaternion& other)
{
    x = x * other.w + w * other.x - z * other.y + y * other.z;
    y = y * other.w + z * other.x + w * other.y - x * other.z;
    z = z * other.w - y * other.x + x * other.y + w * other.z;
    w = w * other.w - x * other.x - y * other.y - z * other.z;

    return *this;
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

inline Vector3 Quaternion::operator*(const Vector3& v) const
{
    Vector3 t(
        2.0f * (y * v.z - z * v.y),
        2.0f * (z * v.x - x * v.z),
        2.0f * (x * v.y - y * v.x)
    );
    
    Vector3 result(
        v.x + w * t.x + (y * t.z - z * t.y),
        v.y + w * t.y + (z * t.x - x * t.z),
        v.z + w * t.z + (x * t.y - y * t.x)
    );

    return result;
}

inline float Quaternion::Dot(const Quaternion& rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
}

inline float Quaternion::GetAngleBetween(const Quaternion& first, const Quaternion& second)
{
    Quaternion qRel = first.GetInverse() * second;
    return 2.0f * GoknarMath::Acos(GoknarMath::Clamp(GoknarMath::Abs(qRel.w), -1.0f, 1.0f));
}

inline Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angleRadians)
{
    Vector3 n = axis;
    n.Normalize();

    const float halfAngle = angleRadians * 0.5f;
    const float sinHalf = GoknarMath::Sin(halfAngle);
    const float cosHalf = GoknarMath::Cos(halfAngle);

    Quaternion q;
    q.x = n.x * sinHalf;
    q.y = n.y * sinHalf;
    q.z = n.z * sinHalf;
    q.w = cosHalf;

    return q;
}

float Quaternion::Length() const
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

inline Quaternion Quaternion::GetNormalized() const
{
    Quaternion q(*this);
    q.Normalize();
    return q;
}

inline Quaternion& Quaternion::Normalize()
{
    const float magnitude = sqrtf(x * x + y * y + z * z + w * w);
    if (0.f < magnitude)
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
    const float squareLength = x * x + y * y + z * z + w * w;

    if (squareLength > 0.0f)
    {
        const float invSqLen = 1.0f / squareLength;
        x = -x * invSqLen;
        y = -y * invSqLen;
        z = -z * invSqLen;
        w = w * invSqLen;
    }

    return *this;
}

#endif