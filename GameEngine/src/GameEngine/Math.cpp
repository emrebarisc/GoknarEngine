/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#include "Math.h"
#include "Matrix.h"

const Vector3 Vector3::ZeroVector = Vector3(0.f);
const Vector2 Vector2::ZeroVector = Vector2(0.f);

float EPSILON = 0.001f;
float INTERSECTION_TEST_EPSILON = 0.001f;
float SHADOW_EPSILON = 0.001f;

Vector2::Vector2(const Vector3& rhs) : x(rhs.x), y(rhs.y)
{
    
}

Vector3::Vector3(const Vector4& rhs) : x(rhs.x), y(rhs.y), z(rhs.z)
{
    
}

Vector4 Vector4::operator*(const Matrix &rhs) const
{
return Vector4(x * rhs.m[0] + y * rhs.m[4] + z * rhs.m[8] + w * rhs.m[12],
                x * rhs.m[1] + y * rhs.m[5] + z * rhs.m[9] + w * rhs.m[13],
                x * rhs.m[2] + y * rhs.m[6] + z * rhs.m[10] + w * rhs.m[14],
                x * rhs.m[3] + y * rhs.m[7] + z * rhs.m[11] + w * rhs.m[15]);
}