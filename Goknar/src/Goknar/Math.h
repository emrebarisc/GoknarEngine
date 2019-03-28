/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#ifndef __MATH_H__
#define __MATH_H__

#define mathMax(f, s) (f > s ? f : s)
#define mathMin(f, s) (f > s ? s : f)
#define mathClamp(value, min, max) (value > max ? max : value < min ? min : value)
#define mathAbs(value) (value < 0 ? value * -1 : value)

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define HALF_PI 1.57079632679
#define ONE_OVER_PI 0.31830988618
#define NATURAL_LOGARITHM 2.71828182845

#define RADIAN_TO_DEGREE(radian) (radian * 180 / PI)
#define DEGREE_TO_RADIAN(degree) (degree * PI / 180)

extern const float MAX_FLOAT;
extern const int MAX_INT;
extern const unsigned int MAX_UINT;

struct Vector2;
struct Vector2i;
struct Vector3;
struct Vector3i;
struct Vector4;

struct Matrix;

struct Vector2
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

struct Vector2i
{
	Vector2i();

	Vector2i(const Vector2& vec2f);

  int x, y;
};

struct Vector3
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

	static inline float Dot(const Vector3& v1, const Vector3& v2);

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

	inline friend std::ostream& operator<<(std::ostream& out, const Vector3& val);

	inline void Clamp(int min, int max);

	Vector3 GetOrthonormalBasis() const;

  static const Vector3 ZeroVector;

  float x, y, z;
};

struct Vector3i
{
	Vector3i();

	Vector3i(const Vector3& rhs);

  int x, y, z;
};

struct Vector4
{
	Vector4();

	Vector4(float value);

	Vector4(float xVal, float yVal, float zVal, float wVal);

  /* Vector4(const Vector4 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
  {

  } */

	Vector4(const Vector3& rhs, float value = 0);

  Vector4 operator*(const Matrix &rhs) const;

	inline friend std::ostream& operator<<(std::ostream& out, const Vector4& val);

  float x, y, z, w;
};

class Math
{
public:
  //static float Determinant(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);

	static inline float Determinant(const Vector3& a, const Vector3& b, const Vector3& c);

	static inline float Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
};

extern float EPSILON;
extern float INTERSECTION_TEST_EPSILON;
extern float SHADOW_EPSILON;

#endif