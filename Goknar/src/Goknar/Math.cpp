/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#include "pch.h"

#include "Math.h"
#include "Matrix.h"

const Vector3 Vector3::ZeroVector = Vector3(0.f);
const Vector2 Vector2::ZeroVector = Vector2(0.f);

/*const float MAX_FLOAT = std::numeric_limits<float>::max();
const int MAX_INT = std::numeric_limits<int>::max();
const unsigned int MAX_UINT = std::numeric_limits<unsigned int>::max();*/

float EPSILON = 0.001f;
float INTERSECTION_TEST_EPSILON = 0.001f;
float SHADOW_EPSILON = 0.001f;

Vector2::Vector2(): x(0), y(0)
{
}

Vector2::Vector2(float val): x(val), y(val)
{
}

Vector2::Vector2(float vx, float vy): x(vx), y(vy)
{
}

float Vector2::Length() const
{
	return sqrt(std::pow(x, 2) + std::pow(y, 2));
}

Vector2 Vector2::Cross(const Vector2& v1, const Vector2& v2)
{
	return Vector2(v1.x * v2.y - v1.y * v2.x);
}

float Vector2::Dot(const Vector2& v1, const Vector2& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

void Vector2::Normalize()
{
	*this /= this->Length();
}

void Vector2::Normalize(Vector2& vec)
{
	float vecLen = vec.Length();
	/*if(vecLen == 0) 
	{
	  //throw std::runtime_error("Error: Vector length is 0. Division by zero.");
	  std::cerr << "Division by zero." << std::endl;
	  return;
	}*/

	vec = vec / vecLen;
}

Vector2 Vector2::GetNormalized() const
{
	return *this / this->Length();
}

Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}

Vector2 Vector2::operator-(const Vector2& rhs) const
{
	return Vector2(x - rhs.x, y - rhs.y);
}

Vector2 Vector2::operator+(const Vector2& rhs) const
{
	return Vector2(x + rhs.x, y + rhs.y);
}

void Vector2::operator+=(const Vector2& rhs)
{
	x = x + rhs.x;
	y = y + rhs.y;
}

Vector2 Vector2::operator*(float val) const
{
	return Vector2(x * val, y * val);
}

Vector2 operator*(float val, const Vector2& rhs)
{
	return Vector2(rhs.x * val, rhs.y * val);
}

Vector2 Vector2::operator*(const Vector2& rhs) const
{
	return Vector2(x * rhs.x, y * rhs.y);
}

Vector2 Vector2::operator/(float val) const
{
	return Vector2(x / val, y / val);
}

Vector2 Vector2::operator/(const Vector2& rhs) const
{
	return Vector2(x / rhs.x, y / rhs.y);
}

void Vector2::operator/=(const float& val)
{
	x /= val;
	y /= val;
}

bool Vector2::operator==(const Vector2& val) const
{
	return x == val.x && y == val.y;
}

bool Vector2::operator!=(const Vector2& val) const
{
	return x != val.x && y != val.y;
}

std::ostream& operator<<(std::ostream& out, const Vector2& val)
{
	return out << "[" << val.x << ", " << val.y << "]";
}

void Vector2::Clamp(float min, float max)
{
	x = mathClamp(x, min, max);
	y = mathClamp(y, min, max);
}

Vector2i::Vector2i(): x(0), y(0)
{
}

Vector2i::Vector2i(const Vector2& vec2f)
{
	x = (int)vec2f.x;
	y = (int)vec2f.y;
}

Vector3::Vector3(): x(0), y(0), z(0)
{
}

Vector3::Vector3(float val): x(val), y(val), z(val)
{
}

Vector3::Vector3(float vx, float vy, float vz): x(vx), y(vy), z(vz)
{
}

float Vector3::Length() const
{
	return sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.y * v2.z - v1.z * v2.y,
	               v1.z * v2.x - v1.x * v2.z,
	               v1.x * v2.y - v1.y * v2.x);
}

float Vector3::Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void Vector3::Normalize()
{
	*this /= this->Length();
}

void Vector3::Normalize(Vector3& vec)
{
	float vecLen = vec.Length();
	/*if(vecLen == 0) 
	{
	  //throw std::runtime_error("Error: Vector length is 0. Division by zero.");
	  std::cerr << "Division by zero." << std::endl;
	  return;
	}*/

	vec = vec / vecLen;
}

Vector3 Vector3::GetNormalized() const
{
	return *this / this->Length();
}

Vector3 Vector3::GetNormalized(const Vector3& vec)
{
	return vec / vec.Length();
}

Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

void Vector3::operator+=(const Vector3& rhs)
{
	x = x + rhs.x;
	y = y + rhs.y;
	z = z + rhs.z;
}

Vector3 Vector3::operator*(float val) const
{
	return Vector3(x * val, y * val, z * val);
}

void Vector3::operator*=(float val)
{
	x *= val;
	y *= val;
	z *= val;
}

Vector3 operator*(float val, const Vector3& rhs)
{
	return Vector3(rhs.x * val, rhs.y * val, rhs.z * val);
}

Vector3 Vector3::operator*(const Vector3& rhs) const
{
	return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
}

Vector3 Vector3::operator/(float val) const
{
	return Vector3(x / val, y / val, z / val);
}

Vector3 Vector3::operator/(const Vector3& rhs) const
{
	return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
}

void Vector3::operator/=(const float& val)
{
	x /= val;
	y /= val;
	z /= val;
}

bool Vector3::operator==(const Vector3& val) const
{
	return x == val.x && y == val.y && z == val.z;
}

bool Vector3::operator!=(const Vector3& val) const
{
	return x != val.x && y != val.y && z != val.z;
}

std::ostream& operator<<(std::ostream& out, const Vector3& val)
{
	return out << "[" << val.x << ", " << val.y << ", " << val.z << "]";
}

void Vector3::Clamp(int min, int max)
{
	x = mathClamp(x, min, max);
	y = mathClamp(y, min, max);
	z = mathClamp(z, min, max);
}

Vector3 Vector3::GetOrthonormalBasis() const
{
	Vector3 maximizedValue = Vector3::ZeroVector;
	if (x < y && x < z)
	{
		maximizedValue = Vector3(1.f, y, z);
	}
	else if (y < x && y < z)
	{
		maximizedValue = Vector3(x, 1.f, z);
	}
	else
	{
		maximizedValue = Vector3(x, y, 1.f);
	}

	Vector3 firstCrossed = Vector3::Cross(maximizedValue, *this);
	Vector3 secondCrossed = Vector3::Cross(firstCrossed, *this);

	return secondCrossed.GetNormalized();
}

Vector3i::Vector3i(): x(0), y(0), z(0)
{
}

Vector3i::Vector3i(const Vector3& rhs)
{
	x = (int)rhs.x;
	y = (int)rhs.y;
	z = (int)rhs.z;
}

Vector4::Vector4(): x(0.f), y(0.f), z(0.f), w(0.f)
{
}

Vector4::Vector4(float value): x(value), y(value), z(value), w(value)
{
}

Vector4::Vector4(float xVal, float yVal, float zVal, float wVal): x(xVal), y(yVal), z(zVal), w(wVal)
{
}

Vector4::Vector4(const Vector3& rhs, float value): x(rhs.x), y(rhs.y), z(rhs.z), w(value)
{
}

std::ostream& operator<<(std::ostream& out, const Vector4& val)
{
	return out << "[" << val.x << ", " << val.y << ", " << val.z << ", " << val.w << "]";
}

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

float Math::Determinant(const Vector3& a, const Vector3& b, const Vector3& c)
{
	return a.x * ((b.y * c.z) - (c.y * b.z))
		+ a.y * ((c.x * b.z) - (b.x * c.z))
		+ a.z * ((b.x * c.y) - (c.x * b.y));
}

float Math::Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
{
	return a.x * Determinant(Vector3(b.y, b.z, b.w), Vector3(c.y, c.z, c.w), Vector3(d.y, d.z, d.w))
		- a.y * Determinant(Vector3(b.x, b.z, b.w), Vector3(c.x, c.z, c.w), Vector3(d.x, d.z, d.w))
		+ a.z * Determinant(Vector3(b.x, b.y, b.w), Vector3(c.x, c.y, c.w), Vector3(d.x, d.y, d.w))
		- a.w * Determinant(Vector3(b.x, b.y, b.z), Vector3(c.x, c.y, c.z), Vector3(d.x, d.y, d.z));
}
