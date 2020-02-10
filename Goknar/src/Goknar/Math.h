#ifndef __MATH_H__
#define __MATH_H__

#include "Goknar/Core.h"

#include <iostream>

#define mathMax(f, s) (f > s ? f : s)
#define mathMin(f, s) (f > s ? s : f)
#define mathClamp(value, min, max) (value > max ? max : value < min ? min : value)
#define mathAbs(value) (value < 0 ? value * -1 : value)

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HALF_PI 1.57079632679f
#define ONE_OVER_PI 0.31830988618f
#define NATURAL_LOGARITHM 2.71828182845f

#define RADIAN_TO_DEGREE(radian) (radian * 180.f / PI)
#define DEGREE_TO_RADIAN(degree) (degree * PI / 180.f)

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

  /* Vector2(const Vector2& rhs) : x(rhs.x), y(rhs.y)
  {

  } */

  Vector2(const Vector3& rhs);

	inline float Length() const;

	static inline Vector2 Cross(const Vector2& v1, const Vector2& v2);

	static inline float Dot(const Vector2& v1, const Vector2& v2);

	void Normalize();

	static inline void Normalize(Vector2& vec);


	Vector2 GetNormalized() const;

	inline Vector2 operator-() const;

	inline Vector2 operator-(const Vector2& rhs) const;

	inline Vector2 operator+(const Vector2& rhs) const;

  /* inline void operator=(const Vector2& rhs)
  {
    x = rhs.x;
    y = rhs.y;
  } */

	inline void operator+=(const Vector2& rhs);

	inline Vector2 operator*(float val) const;

	inline friend Vector2 operator*(float val, const Vector2& rhs);

	inline Vector2 operator*(const Vector2& rhs) const;

	inline Vector2 operator/(float val) const;

	inline Vector2 operator/(const Vector2& rhs) const;

	inline void operator/=(const float& val);

	inline bool operator==(const Vector2& val) const;

	inline bool operator!=(const Vector2& val) const;

	inline friend std::ostream& operator<<(std::ostream& out, const Vector2& val);

	inline void Clamp(float min, float max);

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

	inline Vector2i operator-(const Vector2i& rhs) const;
	inline Vector2i operator+(const Vector2i& rhs) const;

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

	inline float Length() const;

	static inline Vector3 Cross(const Vector3& v1, const Vector3& v2);
	inline Vector3 Cross(const Vector3& rhs) const;

	static inline float Dot(const Vector3& v1, const Vector3& v2);
	inline float Dot(const Vector3& rhs);

	void Normalize();

	static inline void Normalize(Vector3& vec);

	inline Vector3 GetNormalized() const;

	static inline Vector3 GetNormalized(const Vector3& vec);

	inline Vector3 operator-() const;

	inline Vector3 operator-(const Vector3& rhs) const;

	inline Vector3 operator+(const Vector3& rhs) const;

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

	inline void operator+=(const Vector3& rhs);

	inline Vector3 operator*(float val) const;

	inline void operator*=(float val);

	inline friend Vector3 operator*(float val, const Vector3& rhs);

	inline Vector3 operator*(const Vector3& rhs) const;

	inline Vector3 operator/(float val) const;

	inline Vector3 operator/(const Vector3& rhs) const;

	inline void operator/=(const float& val);

	inline bool operator==(const Vector3& val) const;

	inline bool operator!=(const Vector3& val) const;

	inline friend std::ostream& operator<<(std::ostream& out, const Vector3& val)
	{
		out << "[" << val.x << ", " << val.y << ", " << val.z << "]";
		return out;
	}

	inline void Clamp(int min, int max);

	void ConvertDegreeToRadian();
	void ConvertRadianToDegree();

	Vector3 GetOrthonormalBasis() const;

	/*
	*	Transformations
	*/

	Vector3 Translate(const Vector3& translation);
	Vector3 Rotate(const Vector3& rotation);
	Vector3 Scale(const Vector3& scale);

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

	inline friend std::ostream& operator<<(std::ostream& out, const Vector4& val);

	static const Vector4 ZeroVector;
	float x, y, z, w;
};

class GOKNAR_API Math
{
public:
	static void LookAt(Matrix &viewingMatrix, const Vector3& position, const Vector3& target, const Vector3& upVector);

	static inline float Determinant(const Vector3& a, const Vector3& b, const Vector3& c);

	static inline float Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
};

extern float EPSILON;
extern float INTERSECTION_TEST_EPSILON;
extern float SHADOW_EPSILON;

#endif