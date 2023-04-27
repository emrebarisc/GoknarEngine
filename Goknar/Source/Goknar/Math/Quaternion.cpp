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

Quaternion::Quaternion(const Matrix& rotationMatrix)
{
    float t = rotationMatrix[0] + rotationMatrix[5] + rotationMatrix[10];

    if (t > 0.f)
    {
        float s = std::sqrt(1.f + t) * 2.f;
        x = (rotationMatrix[9] - rotationMatrix[6]) / s;
        y = (rotationMatrix[2] - rotationMatrix[8]) / s;
        z = (rotationMatrix[4] - rotationMatrix[1]) / s;
        w = static_cast<float>(0.25) * s;
    }
    else if (rotationMatrix[0] > rotationMatrix[5] && rotationMatrix[0] > rotationMatrix[10])
    {

        float s = std::sqrt(1.f + rotationMatrix[0] - rotationMatrix[5] - rotationMatrix[10]) * 2.f;
        x = 0.25f * s;
        y = (rotationMatrix[4] + rotationMatrix[1]) / s;
        z = (rotationMatrix[2] + rotationMatrix[8]) / s;
        w = (rotationMatrix[9] - rotationMatrix[6]) / s;
    }
    else if (rotationMatrix[5] > rotationMatrix[10])
    {

        float s = std::sqrt(1.f + rotationMatrix[5] - rotationMatrix[0] - rotationMatrix[10]) * 2.f;
        x = (rotationMatrix[4] + rotationMatrix[1]) / s;
        y = 0.25f * s;
        z = (rotationMatrix[9] + rotationMatrix[6]) / s;
        w = (rotationMatrix[2] - rotationMatrix[8]) / s;
    }
    else
    {
        float s = std::sqrt(1.f + rotationMatrix[10] - rotationMatrix[0] - rotationMatrix[5]) * 2.f;
        x = (rotationMatrix[2] + rotationMatrix[8]) / s;
        y = (rotationMatrix[9] + rotationMatrix[6]) / s;
        z = 0.25f * s;
        w = (rotationMatrix[4] - rotationMatrix[1]) / s;
    }
}

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

Quaternion::Quaternion(float roll, float pitch, float yaw)
{
    const float sinPitch(std::sinf(pitch * 0.5f));
    const float cosPitch(std::cosf(pitch * 0.5f));
    const float sinYaw(std::sinf(yaw * 0.5f));
    const float cosYaw(std::cosf(yaw * 0.5f));
    const float sinRoll(std::sinf(roll * 0.5f));
    const float cosRoll(std::cosf(roll * 0.5f));
    const float cosPitchCosYaw(cosPitch * cosYaw);
    const float sinPitchSinYaw(sinPitch * sinYaw);
    //x = cosRoll * sinPitchSinYaw - sinRoll * cosPitchCosYaw;
    //y = -cosRoll * sinPitch * cosYaw - sinRoll * cosPitch * sinYaw;
    //z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
    //w = cosRoll * cosPitchCosYaw + sinRoll * sinPitchSinYaw;
    x = sinRoll * cosPitchCosYaw - cosRoll * sinPitchSinYaw;
    y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
    z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
    w = cosRoll * cosPitchCosYaw + sinRoll * sinPitchSinYaw;
}

Quaternion::Quaternion(Vector3 axis, float angle)
{
    axis.Normalize();
    const float halfAngle = angle * 0.5f;
    const float sinAngle = std::sin(halfAngle);
    x = axis.x * sinAngle;
    y = axis.y * sinAngle;
    z = axis.z * sinAngle;
    w = std::cos(halfAngle);
}

Quaternion Quaternion::FromEuler(const Vector3& degrees)
{
    return FromEulerRadians(degrees * TO_RADIAN);
}

Quaternion Quaternion::FromEulerRadians(const Vector3& radians)
{
    return Quaternion(
        std::fmodf(radians.x, TWO_PI),
        std::fmodf(radians.y, TWO_PI),
        std::fmodf(radians.z, TWO_PI)
    );
}

inline bool Quaternion::Equals(const Quaternion& other, float tolerance) const
{
    return  std::abs(x - other.x) <= tolerance &&
            std::abs(y - other.y) <= tolerance &&
            std::abs(z - other.z) <= tolerance &&
            std::abs(w - other.w) <= tolerance;
}

Vector3 Quaternion::ToEuler() const
{
    return ToEulerRadians() * TO_DEGREE;
}

Vector3 Quaternion::ToEulerRadians() const
{
    Vector3 euler;

    // Roll
    const float sinRCosP = 2.f * (w * x + y * z);
    const float cosRCosP = 1.f - 2.f * (x * x + y * y);
    euler.x = std::atan2f(sinRCosP, cosRCosP);

    // Pitch
    const float sinP = std::sqrtf(1.f + 2.f * (w * y - x * z));
    const float cosP = std::sqrtf(1.f - 2.f * (w * y - x * z));
    euler.y = 2.f * std::atan2f(sinP, cosP) - HALF_PI;

    // Yaw
    float sinYCosP = 2.f * (w * z + x * y);
    float cosYCosP = 1.f - 2.f * (y * y + z * z);
    euler.z = std::atan2f(sinYCosP, cosYCosP);

    return euler;
}

inline Matrix Quaternion::GetMatrix() const
{
    return Matrix(
        1.f - 2.f * (y * y + z * z),    2.f * (x * y - z * w),          2.f * (x * z + y * w),          0.f,
        2.f * (x * y + z * w),          1.f - 2.f * (x * x + z * z),    2.f * (y * z - x * w),          0.f,
        2.f * (x * z - y * w),          2.f * (y * z + x * w),          1.f - 2.f * (x * x + y * y),    0.f,
        0.f,                            0.f,                            0.f,                            1.f
    );
}

inline Matrix3x3 Quaternion::GetMatrix3x3() const
{
    return Matrix3x3(
        1.f - 2.f * (y * y + z * z),    2.f * (x * y - z * w),          2.f * (x * z + y * w),
        2.f * (x * y + z * w),          1.f - 2.f * (x * x + z * z),    2.f * (y * z - x * w),
        2.f * (x * z - y * w),          2.f * (y * z + x * w),          1.f - 2.f * (x * x + y * y)
    );
}

Quaternion Quaternion::Pow(float n)
{
    return (Ln() * n).Exp();
}

Quaternion Quaternion::Exp()
{
    Quaternion value(*this);

    float r = std::sqrtf(value.x * value.x + value.y * value.y + value.z * value.z);
    float et = std::expf(value.w);
    float s = r >= 0.00001f ? et * std::sinf(r) / r : 0.f;

    value.w = et * std::cosf(r);
    value.x *= s;
    value.y *= s;
    value.z *= s;
    return value;
}

Quaternion Quaternion::Ln()
{
    Quaternion value(*this);
    float r = std::sqrtf(value.x * value.x + value.y * value.y + value.z * value.z);
    float t = r > 0.00001f ? std::atan2f(r, value.w) / r : 0.f;
    value.w = 0.5f * std::logf(value.w * value.w + value.x * value.x + value.y * value.y + value.z * value.z);
    value.x *= t;
    value.y *= t;
    value.z *= t;
    return value;
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

inline void Quaternion::Slerp(Quaternion& out, const Quaternion& start, const Quaternion& end, float alpha)
{
    float cosom = start.x * end.x + start.y * end.y + start.z * end.z + start.w * end.w;

    Quaternion correctedEnd = end;
    if (cosom < 0.f)
    {
        cosom = -cosom;
        correctedEnd.x = -correctedEnd.x;
        correctedEnd.y = -correctedEnd.y;
        correctedEnd.z = -correctedEnd.z;
        correctedEnd.w = -correctedEnd.w;
    }

    float sclp, sclq;
    if ((1.0f - cosom) > EPSILON)
    {
        float omega, sinom;
        omega = std::acos(cosom);
        sinom = std::sin(omega);
        sclp = std::sin((1.0f - alpha) * omega) / sinom;
        sclq = std::sin(alpha * omega) / sinom;
    }
    else
    {
        sclp = 1.0f - alpha;
        sclq = alpha;
    }

    out.x = sclp * start.x + sclq * correctedEnd.x;
    out.y = sclp * start.y + sclq * correctedEnd.y;
    out.z = sclp * start.z + sclq * correctedEnd.z;
    out.w = sclp * start.w + sclq * correctedEnd.w;
}
