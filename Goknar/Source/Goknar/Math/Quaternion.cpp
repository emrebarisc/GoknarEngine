#include "pch.h"

#include "Quaternion.h"

#include "GoknarMath.h"
#include "Matrix.h"

const Quaternion Quaternion::Identity = Quaternion(0.f, 0.f, 0.f, 1.f);

Quaternion::Quaternion(const Vector4& v) :
    x(v.x),
    y(v.y),
    z(v.z),
    w(v.w)
{}

Quaternion::Quaternion(const Matrix3x3& rotationMatrix)
{
    float t = rotationMatrix[0] + rotationMatrix[4] + rotationMatrix[8];

    if (t > 0.f)
    {
        float s = std::sqrt(1.f + t) * 2.f;
        x = (rotationMatrix[7] - rotationMatrix[5]) / s;
        y = (rotationMatrix[2] - rotationMatrix[6]) / s;
        z = (rotationMatrix[3] - rotationMatrix[1]) / s;
        w = static_cast<float>(0.25) * s;
    }
    else if (rotationMatrix[0] > rotationMatrix[4] && rotationMatrix[0] > rotationMatrix[8])
    {

        float s = std::sqrt(1.f + rotationMatrix[0] - rotationMatrix[4] - rotationMatrix[8]) * 2.f;
        x = 0.25f * s;
        y = (rotationMatrix[3] + rotationMatrix[1]) / s;
        z = (rotationMatrix[2] + rotationMatrix[6]) / s;
        w = (rotationMatrix[7] - rotationMatrix[5]) / s;
    }
    else if (rotationMatrix[4] > rotationMatrix[8])
    {

        float s = std::sqrt(1.f + rotationMatrix[4] - rotationMatrix[0] - rotationMatrix[8]) * 2.f;
        x = (rotationMatrix[3] + rotationMatrix[1]) / s;
        y = 0.25f * s;
        z = (rotationMatrix[7] + rotationMatrix[5]) / s;
        w = (rotationMatrix[2] - rotationMatrix[6]) / s;
    }
    else
    {
        float s = std::sqrt(1.f + rotationMatrix[8] - rotationMatrix[0] - rotationMatrix[4]) * 2.f;
        x = (rotationMatrix[2] + rotationMatrix[6]) / s;
        y = (rotationMatrix[7] + rotationMatrix[5]) / s;
        z = 0.25f * s;
        w = (rotationMatrix[3] - rotationMatrix[1]) / s;
    }
}

Quaternion::Quaternion(float pitch, float yaw, float roll)
{
    const float sinPitch(std::sin(pitch * 0.5f));
    const float cosPitch(std::cos(pitch * 0.5f));
    const float sinYaw(std::sin(yaw * 0.5f));
    const float cosYaw(std::cos(yaw * 0.5f));
    const float sinRoll(std::sin(roll * 0.5f));
    const float cosRoll(std::cos(roll * 0.5f));
    const float cosPitchCosYaw(cosPitch * cosYaw);
    const float sinPitchSinYaw(sinPitch * sinYaw);
    x = sinRoll * cosPitchCosYaw - cosRoll * sinPitchSinYaw;
    y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
    z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
    w = cosRoll * cosPitchCosYaw + sinRoll * sinPitchSinYaw;
}

Quaternion::Quaternion(Vector3 axis, float angle)
{
    axis.GetNormalized();

    const float sinAngle = std::sin(angle * 0.5f);
    const float cosAngle = std::cos(angle * 0.5f);
    x = axis.x * sinAngle;
    y = axis.y * sinAngle;
    z = axis.z * sinAngle;
    w = cosAngle;
}

Quaternion::Quaternion(const Vector3& normalized)
{
    x = normalized.x;
    y = normalized.y;
    z = normalized.z;

    const float t = 1.f - (x * x) - (y * y) - (z * z);

    if (t < 0.f)
    {
        w = 0.f;
    }
    else
    {
        w = std::sqrt(t);
    }
}

inline bool Quaternion::Equals(const Quaternion& other, float tolerance) const
{
    return  std::abs(x - other.x) <= tolerance &&
            std::abs(y - other.y) <= tolerance &&
            std::abs(z - other.z) <= tolerance &&
            std::abs(w - other.w) <= tolerance;
}

inline Matrix3x3 Quaternion::GetMatrix() const
{
    return Matrix3x3(
        1.f - 2.f * (y * y + z * z), 2.f * (x * y - z * w), 2.f * (x * z + y * w),
        2.f * (x * y + z * w), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - x * w),
        2.f * (x * z - y * w), 2.f * (y * z + x * w), 1.f - 2.f * (x * x + y * y)
    );
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

inline Quaternion& Quaternion::Conjugate()
{
    x = -x;
    y = -y;
    z = -z;
    return *this;
}

inline Vector3 Quaternion::Rotate(const Vector3& v) const
{
    Quaternion q2(0.f, v.x, v.y, v.z);
    Quaternion q = *this;
    Quaternion qInverse = q.Conjugate();

    q = q * q2 * qInverse;
    return Vector3(q.x, q.y, q.z);
}

Quaternion operator*(const Matrix& pMatrix, const Quaternion& quaternion)
{
    Quaternion res;
    res.x = pMatrix[0] * quaternion.x + pMatrix[1] * quaternion.y + pMatrix[2] * quaternion.z + pMatrix[3] * quaternion.w;
    res.y = pMatrix[4] * quaternion.x + pMatrix[5] * quaternion.y + pMatrix[6] * quaternion.z + pMatrix[7] * quaternion.w;
    res.z = pMatrix[8] * quaternion.x + pMatrix[9] * quaternion.y + pMatrix[10] * quaternion.z + pMatrix[11] * quaternion.w;
    res.w = pMatrix[12] * quaternion.x + pMatrix[13] * quaternion.y + pMatrix[14] * quaternion.z + pMatrix[15] * quaternion.w;
    return res;
}
inline Quaternion Quaternion::operator*(const Matrix& scaleMatrix) const
{
    return scaleMatrix * (*this);
}

inline Quaternion& Quaternion::operator*=(const Matrix& scaleMatrix)
{
    return (*this = scaleMatrix * (*this));
}