#include "pch.h"

#include "Math.h"
#include "Matrix.h"

const Vector2 Vector2::ZeroVector = Vector2(0.f);

const Vector3 Vector3::ZeroVector = Vector3(0.f);
const Vector3 Vector3::ForwardVector = Vector3(1.f, 0.f, 0.f);
const Vector3 Vector3::LeftVector = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::UpVector = Vector3(0.f, 0.f, 1.f);

const Vector4 Vector4::ZeroVector = Vector4(0.f);

//constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
//constexpr int MAX_INT = std::numeric_limits<int>::max();
//constexpr unsigned int MAX_UINT = std::numeric_limits<unsigned int>::max();

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

Vector2i::Vector2i(int vx, int vy) : x(vx), y(vy)
{
}

Vector2i::Vector2i(const Vector2& vec2f)
{
	x = (int)vec2f.x;
	y = (int)vec2f.y;
}

inline Vector2i Vector2i::operator-(const Vector2i& rhs) const
{
	return Vector2i(x - rhs.x, y - rhs.y);
}

inline Vector2i Vector2i::operator+(const Vector2i& rhs) const
{
	return Vector2i(x + rhs.x, y + rhs.y);
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

inline Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.y * v2.z - v1.z * v2.y,
	               v1.z * v2.x - v1.x * v2.z,
	               v1.x * v2.y - v1.y * v2.x);
}

Vector3 Vector3::Cross(const Vector3& rhs) const
{
	return Vector3(y * rhs.z - z * rhs.y,
				   z * rhs.x - x * rhs.z,
				   x * rhs.y - y * rhs.x);
}

inline float Vector3::Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vector3::Dot(const Vector3& rhs)
{
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

void Vector3::Normalize()
{
	*this /= this->Length();
}

void Vector3::Normalize(Vector3& vec)
{
	try
	{
		float vecLen = vec.Length();
		vec = vec / vecLen;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception thrown: " << e.what() << std::endl;
	}
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

void Vector3::Clamp(int min, int max)
{
	x = mathClamp(x, min, max);
	y = mathClamp(y, min, max);
	z = mathClamp(z, min, max);
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

Vector3 Vector3::Translate(const Vector3& translation)
{
	Matrix translateMatrix = Matrix::IdentityMatrix;

	translateMatrix.m[3] = translation.x;
	translateMatrix.m[7] = translation.y;
	translateMatrix.m[11] = translation.z;

	return Vector3(translateMatrix * Vector4(translation));
}

Vector3 Vector3::Rotate(const Vector3& rotation)
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
		result = Vector3(rotateMatrix * Vector4(result));
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
		result = Vector3(rotateMatrix * Vector4(result));
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
		result = Vector3(rotateMatrix * Vector4(result));
	}

	return result;
}

Vector3 Vector3::Scale(const Vector3& scale)
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

void Math::LookAt(Matrix& viewingMatrix, const Vector3& position, const Vector3& target, const Vector3& upVector)
{   
	Vector3 forward = position - target;
	forward.Normalize();

	Vector3 left = upVector.Cross(forward);
	left.Normalize();

	Vector3 up = forward.Cross(left);

	viewingMatrix = Matrix(left.x, up.x, forward.x, 0.f,
		left.y, up.y, forward.y, 0.f,
		left.z, up.z, forward.z, 0.f,
		-left.x * position.x - left.y * position.y - left.z * position.z, -up.x * position.x - up.y * position.y - up.z * position.z, -forward.x * position.x - forward.y * position.y - forward.z * position.z, 1.f);
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
