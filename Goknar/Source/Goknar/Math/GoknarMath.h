#ifndef __MATH_H__
#define __MATH_H__

#include "Goknar/Core.h"
#include "MathDefines.h"

#include "Quaternion.h"

#include <iostream>

#define mathMax(f, s) (f > s ? f : s)
#define mathMin(f, s) (f > s ? s : f)
#define mathClamp(value, min, max) (value > max ? max : value < min ? min : value)
#define mathAbs(value) (value < 0 ? value * -1 : value)

extern const float MAX_FLOAT;
extern const int MAX_INT;
extern const unsigned int MAX_UINT;

struct Vector2;
struct Vector2i;
struct Vector3;
struct Vector3i;
struct Vector4;

class Matrix;

struct GOKNAR_API Vector2
{
	Vector2();
	Vector2(float val);
	Vector2(float vx, float vy);
	Vector2(const Vector3& rhs);

	inline float Length() const
	{
		return sqrt(std::pow(x, 2) + std::pow(y, 2));
	}

	static inline Vector2 Cross(const Vector2& v1, const Vector2& v2)
	{
		return Vector2(v1.x * v2.y - v1.y * v2.x);
	}

	static inline float Dot(const Vector2& v1, const Vector2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	void Normalize();

	static inline void Normalize(Vector2& vec)
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

	Vector2 GetNormalized() const;

	inline Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}

	inline Vector2 operator-(const Vector2& rhs) const
	{
		return Vector2(x - rhs.x, y - rhs.y);
	}

	inline Vector2 operator+(const Vector2& rhs) const
	{
		return Vector2(x + rhs.x, y + rhs.y);
	}

  /* inline void operator=(const Vector2& rhs)
  {
    x = rhs.x;
    y = rhs.y;
  } */

	inline void operator+=(const Vector2& rhs)
	{
		x = x + rhs.x;
		y = y + rhs.y;
	}


	inline Vector2 operator*(float val) const
	{
		return Vector2(x * val, y * val);
	}

	inline friend Vector2 operator*(float val, const Vector2& rhs)
	{
		return Vector2(rhs.x * val, rhs.y * val);
	}

	inline Vector2 operator*(const Vector2& rhs) const
	{
		return Vector2(x * rhs.x, y * rhs.y);
	}

	inline Vector2 operator/(float val) const
	{
		return Vector2(x / val, y / val);
	}

	inline Vector2 operator/(const Vector2& rhs) const
	{
		return Vector2(x / rhs.x, y / rhs.y);
	}

	inline void operator/=(const float& val)
	{
		x /= val;
		y /= val;
	}

	inline bool operator==(const Vector2& val) const
	{
		return x == val.x && y == val.y;
	}

	inline bool operator!=(const Vector2& val) const
	{
		return x != val.x && y != val.y;
	}

	inline friend std::ostream& operator<<(std::ostream& out, const Vector2& val)
	{
		return out << "[" << val.x << ", " << val.y << "]";
	}

	inline std::string ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}

	inline void Clamp(float min, float max)
	{
		x = mathClamp(x, min, max);
		y = mathClamp(y, min, max);
	}

  // Vector2 GetOrthonormalBasis() const 
  // {
  // }

  static const Vector2 ZeroVector;

  float x, y;
};

struct GOKNAR_API Vector2i
{
	Vector2i();
	Vector2i(int vx, int vy);

	Vector2i(const Vector2& vec2f);

	inline Vector2i operator-(const Vector2i& rhs) const
	{
		return Vector2i(x - rhs.x, y - rhs.y);
	}

	inline Vector2i operator+(const Vector2i& rhs) const
	{
		return Vector2i(x + rhs.x, y + rhs.y);
	}

	inline friend Vector2 operator*(const Vector2i vector2i, float multiplier)
	{
		return Vector2(vector2i.x * multiplier, vector2i.y * multiplier);
	}

	inline std::string ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}

	int x, y;
};

struct GOKNAR_API Vector3
{
	Vector3();

	Vector3(float val);

	Vector3(float vx, float vy, float vz);

	/*   Vector3(const Vector3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z)
	{

	} */

	Vector3(const Vector4& rhs);

	inline float Length() const
	{
		return sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
	}

	static inline float Distance(const Vector3& vec1, const Vector3& vec2)
	{
		return (vec2 - vec1).Length();
	}

	inline float Distance(const Vector3& rhs) const
	{
		return (*this - rhs).Length();
	}

	static inline Vector3 Cross(const Vector3& v1, const Vector3& v2)
	{
		return Vector3( v1.y * v2.z - v1.z * v2.y,
						v1.z * v2.x - v1.x * v2.z,
						v1.x * v2.y - v1.y * v2.x);
	}

	inline Vector3 Cross(const Vector3& rhs) const
	{
		return Vector3( y * rhs.z - z * rhs.y,
						z * rhs.x - x * rhs.z,
						x * rhs.y - y * rhs.x);
	}

	static inline float Dot(const Vector3& v1, const Vector3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	inline float Dot(const Vector3& rhs)
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	void Normalize();

	static inline void Normalize(Vector3& vec)
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

	inline Vector3 GetNormalized() const
	{
		float length = this->Length();

		if (length != 0.f)
		{
			return *this / this->Length();
		}

		return *this;
	}

	static inline Vector3 GetNormalized(const Vector3& vec)
	{
		return vec / vec.Length();
	}

	inline Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	inline Vector3 operator-(const Vector3& rhs) const
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	inline Vector3 operator+(const Vector3& rhs) const
	{
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	/*   inline Vector3& operator=(const Vector3& rhs)
	{
	if (this == &rhs) 
	{
		return *this;
	}
    
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
	} */

	inline void operator+=(const Vector3& rhs)
	{
		x = x + rhs.x;
		y = y + rhs.y;
		z = z + rhs.z;
	}

	inline void operator-=(const Vector3& rhs)
	{
		x = x - rhs.x;
		y = y - rhs.y;
		z = z - rhs.z;
	}

	inline Vector3 operator*(float val) const
	{
		return Vector3(x * val, y * val, z * val);
	}

	inline void operator*=(float val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	inline friend Vector3 operator*(float val, const Vector3& rhs)
	{
		return Vector3(rhs.x * val, rhs.y * val, rhs.z * val);
	}

	inline Vector3 operator*(const Vector3& rhs) const
	{
		return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	inline Vector3 operator/(float val) const
	{
		return Vector3(x / val, y / val, z / val);
	}

	inline Vector3 operator/(const Vector3& rhs) const
	{
		return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
	}

	inline void operator/=(const float& val)
	{
		x /= val;
		y /= val;
		z /= val;
	}

	inline bool operator==(const Vector3& val) const
	{
		return x == val.x && y == val.y && z == val.z;
	}

	inline bool operator!=(const Vector3& val) const
	{
		return x != val.x && y != val.y && z != val.z;
	}

	inline friend std::ostream& operator<<(std::ostream& out, const Vector3& val)
	{
		out << "[" << val.x << ", " << val.y << ", " << val.z << "]";
		return out;
	}

	inline std::string ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
	}

	inline void Clamp(int min, int max)
	{
		x = mathClamp(x, min, max);
		y = mathClamp(y, min, max);
		z = mathClamp(z, min, max);
	}

	void ConvertDegreeToRadian();
	void ConvertRadianToDegree();

	Vector3 GetOrthonormalBasis() const;

	Vector3 GetRotation();

	/*
	*	Transformations
	*/

	Vector3 Translate(const Vector3& translation, bool isPositionVector = true) const;
	Vector3 Rotate(const Vector3& rotation, bool isPositionVector = true) const;
	Vector3 Scale(const Vector3& scale, bool isPositionVector = true) const;

	static const Vector3 ZeroVector;
	static const Vector3 ForwardVector;
	static const Vector3 LeftVector;
	static const Vector3 UpVector;

	float x, y, z;
};

struct GOKNAR_API Vector3i
{
	Vector3i();
	Vector3i(const Vector3& rhs);

	inline std::string ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
	}

	int x, y, z;
};

struct GOKNAR_API Vector4
{
	Vector4();

	Vector4(float value);

	Vector4(float xVal, float yVal, float zVal, float wVal);

	Vector4(const Vector4 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{

	}

	Vector4(const Vector3& rhs, float value = 0);
	Vector4 operator*(const Matrix &rhs) const;

	inline friend std::ostream& operator<<(std::ostream& out, const Vector4& val)
	{
		return out << "[" << val.x << ", " << val.y << ", " << val.z << ", " << val.w << "]";
	}

	inline std::string ToString() const
	{
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]";
	}

	static const Vector4 ZeroVector;
	float x, y, z, w;
};

class GOKNAR_API GoknarMath
{
public:
	static void LookAt(Matrix &viewingMatrix, const Vector3& position, const Vector3& target, const Vector3& upVector);

	static inline float Determinant(const Vector3& a, const Vector3& b, const Vector3& c)
	{
		return a.x * ((b.y * c.z) - (c.y * b.z))
			 + a.y * ((c.x * b.z) - (b.x * c.z))
			 + a.z * ((b.x * c.y) - (c.x * b.y));
	}

	static inline float Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
	{
		return	  a.x * Determinant(Vector3(b.y, b.z, b.w), Vector3(c.y, c.z, c.w), Vector3(d.y, d.z, d.w))
				- a.y * Determinant(Vector3(b.x, b.z, b.w), Vector3(c.x, c.z, c.w), Vector3(d.x, d.z, d.w))
				+ a.z * Determinant(Vector3(b.x, b.y, b.w), Vector3(c.x, c.y, c.w), Vector3(d.x, d.y, d.w))
				- a.w * Determinant(Vector3(b.x, b.y, b.z), Vector3(c.x, c.y, c.z), Vector3(d.x, d.y, d.z));
	}

	template<class T>
	static T Lerp(const T& start, const T& end, float alpha)
	{
		return start + alpha * (end - start);
	}

	template<class T>
	static T SlerpIn(const T& start, const T& end, float alpha)
	{
		float const modifiedAlpha = -1.f * (std::sqrt(1.f - alpha * alpha) - 1.f);
		return Lerp(start, end, modifiedAlpha);
	}

	template<class T>
	static T SlerpOut(const T& start, const T& end, float alpha)
	{
		alpha -= 1.f;
		float const modifiedAlpha = std::sqrt(1.f - alpha * alpha);
		return Lerp(start, end, modifiedAlpha);
	}

	template<class T>
	static T Slerp(const T& start, const T& end, float alpha)
	{
		return Lerp(start, end, (alpha < 0.5f) ?
			SlerpIn(0.f, 1.f, alpha * 2.f) * 0.5f :
			SlerpOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f + 0.5f);
	}

	template<class T>
	T Pow(T value, float power)
	{
		return std::pow(value, power);
	}

	/*static Quaternion SphericalInterpolationQ(Quaternion start, Quaternion end, float alpha)
	{
		Quaternion result;

		float cosTheta = start.x * end.x + start.y * end.y + start.z * end.z + start.w * end.w;

		if (cosTheta < 0.f)
		{
			cosTheta = -cosTheta;
			end.x = -end.x;
			end.y = -end.y;
			end.z = -end.z;
			end.w = -end.w;
		}

		float coefficientP, inverseCoefficientP;

		if (EPSILON < (1.f - cosTheta))
		{
			float omega = std::acos(cosTheta);
			float sinOmega = std::sin(omega);
			coefficientP = std::sin((1.f - alpha) * omega) / sinOmega;
			inverseCoefficientP = std::sin(alpha * omega) / sinOmega;
		}
		else
		{
			coefficientP = 1.f - alpha;
			inverseCoefficientP = alpha;
		}

		result.x = coefficientP * start.x + inverseCoefficientP * end.x;
		result.y = coefficientP * start.y + inverseCoefficientP * end.y;
		result.z = coefficientP * start.z + inverseCoefficientP * end.z;
		result.w = coefficientP * start.w + inverseCoefficientP * end.w;

		return result;
	}*/

};

#endif