#include "pch.h"

#include <limits>

#include "GoknarMath.h"
#include "Matrix.h"

const Vector2 Vector2::ZeroVector = Vector2(0.f);

const Vector3 Vector3::ZeroVector = Vector3(0.f);
const Vector3 Vector3::ForwardVector = Vector3(1.f, 0.f, 0.f);
const Vector3 Vector3::LeftVector = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::UpVector = Vector3(0.f, 0.f, 1.f);

const Vector4 Vector4::ZeroVector = Vector4(0.f);

#ifdef GOKNAR_PLATFORM_WINDOWS
const float MAX_FLOAT = FLT_MAX;
const int MAX_INT = INT_MAX;
const unsigned int MAX_UINT = UINT_MAX;

const float MIN_FLOAT = FLT_MIN;
const int MIN_INT = INT_MIN;
#else
const float MAX_FLOAT = std::numeric_limits<float>::max();
const int MAX_INT = std::numeric_limits<int>::max();
const unsigned int MAX_UINT = std::numeric_limits<unsigned int>::max();

const float MIN_FLOAT = std::numeric_limits<float>::min();
const int MIN_INT = std::numeric_limits<int>::min();
#endif // GOKNAR_PLATFORM_WINDOWS

Vector2::Vector2(): x(0), y(0)
{
}

Vector2::Vector2(float val): x(val), y(val)
{
}

Vector2::Vector2(float vx, float vy): x(vx), y(vy)
{
}

void Vector2::Normalize()
{
	*this /= this->Length();
}

Vector2 Vector2::GetNormalized() const
{
	return *this / this->Length();
}

Vector2i::Vector2i(): x(0), y(0)
{
}

Vector2i::Vector2i(int vx, int vy) : x(vx), y(vy)
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

void Vector3::Normalize()
{
	float length = this->Length();
	if (0.f < length)
	{
		*this /= length;
	}
}

void Vector3::ConvertDegreeToRadian()
{
	x = DEGREE_TO_RADIAN(x);
	y = DEGREE_TO_RADIAN(y);
	z = DEGREE_TO_RADIAN(z);
}

void Vector3::ConvertRadianToDegree()
{
	x = RADIAN_TO_DEGREE(x);
	y = RADIAN_TO_DEGREE(y);
	z = RADIAN_TO_DEGREE(z);
}

Vector3 Vector3::GetOrthonormalBasis() const
{
	if (this->SquareLength() < SMALLER_EPSILON)
	{
		// If this is < 0, 0, 0 > then return forward vector as orthonormal basis
		return Vector3::ForwardVector;
	}

	Vector3 maximizedValue = Vector3::ZeroVector;
	if (abs(x) <= abs(y) && abs(x) <= abs(z))
	{
		maximizedValue = Vector3(x < 0 ? -1.f : 1.f, y, z);
	}
	else if (abs(y) <= abs(x) && abs(y) <= abs(z))
	{
		maximizedValue = Vector3(x, y < 0 ? -1.f : 1.f, z);
	}
	else
	{
		maximizedValue = Vector3(x, y, z < 0 ? -1.f : 1.f);
	}

	Vector3 firstCrossed = Vector3::Cross(maximizedValue, *this);
	Vector3 secondCrossed = Vector3::Cross(firstCrossed, *this);

	return secondCrossed.GetNormalized();
}

Quaternion Vector3::GetRotation()
{
	return Quaternion(-atan2(z, sqrt(x * x + y * y)) * TO_RADIAN, atan2(y, x) * TO_RADIAN, 0.f);
}

Quaternion Vector3::GetRotationNormalized()
{
	return Quaternion(0.f, -atan2(z, sqrt(x * x + y * y)),  atan2(y, x));
}

Vector3 Vector3::Translate(const Vector3& translation, bool isPositionVector/* = true*/) const
{
	Matrix translateMatrix = Matrix::IdentityMatrix;

	translateMatrix.m[3] = translation.x;
	translateMatrix.m[7] = translation.y;
	translateMatrix.m[11] = translation.z;

	return Vector3(translateMatrix * Vector4(translation, isPositionVector ? 1.f : 0.f));
}

Vector3 Vector3::Rotate(const Vector3& rotation, bool isPositionVector/* = true*/) const
{
	Vector3 result = *this;

	if (rotation.x != 0)
	{
		Matrix rotateMatrix = Matrix::IdentityMatrix;
		float cosTheta = cos(rotation.x);
		float sinTheta = sin(rotation.x);
		rotateMatrix.m[5] = cosTheta;
		rotateMatrix.m[6] = -sinTheta;
		rotateMatrix.m[9] = sinTheta;
		rotateMatrix.m[10] = cosTheta;
		result = Vector3(rotateMatrix * Vector4(result, isPositionVector ? 1.f : 0.f));
	}
	if (rotation.y != 0)
	{
		Matrix rotateMatrix = Matrix::IdentityMatrix;
		float cosTheta = cos(rotation.y);
		float sinTheta = sin(rotation.y);
		rotateMatrix.m[0] = cosTheta;
		rotateMatrix.m[2] = sinTheta;
		rotateMatrix.m[8] = -sinTheta;
		rotateMatrix.m[10] = cosTheta;
		result = Vector3(rotateMatrix * Vector4(result, isPositionVector ? 1.f : 0.f));
	}
	if (rotation.z != 0)
	{
		Matrix rotateMatrix = Matrix::IdentityMatrix;
		float cosTheta = cos(rotation.z);
		float sinTheta = sin(rotation.z);
		rotateMatrix.m[0] = cosTheta;
		rotateMatrix.m[1] = -sinTheta;
		rotateMatrix.m[4] = sinTheta;
		rotateMatrix.m[5] = cosTheta;
		result = Vector3(rotateMatrix * Vector4(result, isPositionVector ? 1.f : 0.f));
	}

	return result;
}

Vector3 Vector3::RotatePoint(const Vector3& rotation) const
{
	return Rotate(rotation, true);
}

Vector3 Vector3::RotateVector(const Vector3& rotation) const
{
	return Rotate(rotation, false);
}

Vector3 Vector3::RotatePointAroundAxis(const Vector3& axis, float angle)
{
	Matrix rotationMatrix = Matrix::GetRotationMatrixAboutAnAxis(axis, angle);

	Vector3 result = rotationMatrix * Vector4(*this, 1.f);
	return result;
}

Vector3 Vector3::RotateVectorAroundAxis(const Vector3& axis, float angle)
{
	Matrix rotationMatrix = Matrix::GetRotationMatrixAboutAnAxis(axis, angle);

	Vector3 result = rotationMatrix * Vector4(*this, 0.f);
	return result;
}

Vector3 Vector3::Scale(const Vector3& scale, bool isPositionVector/* = true*/) const
{
	Matrix scaleMatrix = Matrix::IdentityMatrix;
	scaleMatrix.m[0] = scale.x;
	scaleMatrix.m[5] = scale.y;
	scaleMatrix.m[10] = scale.z;
	return Vector3(scaleMatrix * Vector4(*this));
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

Vector4::Vector4(const Vector3& rhs, float value) : x(rhs.x), y(rhs.y), z(rhs.z), w(value)
{
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

void GoknarMath::LookAt(Matrix& viewMatrix, const Vector3& position, const Vector3& target, const Vector3& upVector)
{   
	Vector3 forward = position - target;
	forward.Normalize();

	Vector3 left = upVector.Cross(forward);
	left.Normalize();

	Vector3 up = forward.Cross(left);

	viewMatrix = Matrix(	left.x,			left.y,				left.z,				-left.x * position.x - left.y * position.y - left.z * position.z,
							up.x,			up.y,				up.z,				-up.x * position.x - up.y * position.y - up.z * position.z,
							forward.x,		forward.y,			forward.z,			-forward.x * position.x - forward.y * position.y - forward.z * position.z,
							0.f, 0.f, 0.f, 1.f);
}

inline float GoknarMath::Determinant(const Vector3& a, const Vector3& b, const Vector3& c)
{
	return a.x * ((b.y * c.z) - (c.y * b.z))
		+ a.y * ((c.x * b.z) - (b.x * c.z))
		+ a.z * ((b.x * c.y) - (c.x * b.y));
}

inline float GoknarMath::Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
{
	return	  a.x * Determinant(Vector3(b.y, b.z, b.w), Vector3(c.y, c.z, c.w), Vector3(d.y, d.z, d.w))
		- a.y * Determinant(Vector3(b.x, b.z, b.w), Vector3(c.x, c.z, c.w), Vector3(d.x, d.z, d.w))
		+ a.z * Determinant(Vector3(b.x, b.y, b.w), Vector3(c.x, c.y, c.w), Vector3(d.x, d.y, d.w))
		- a.w * Determinant(Vector3(b.x, b.y, b.z), Vector3(c.x, c.y, c.z), Vector3(d.x, d.y, d.z));
}
