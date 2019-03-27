/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#ifndef __MATH_H__
#define __MATH_H__

#include <iostream>
#include <cmath>
#include <stdexcept>

#include <limits>

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

const float MAX_FLOAT = std::numeric_limits<float>::max();
const int MAX_INT = std::numeric_limits<int>::max();
const unsigned int MAX_UINT = std::numeric_limits<unsigned int>::max();

struct Vector2;
struct Vector2i;
struct Vector3;
struct Vector3i;
struct Vector4;

struct Matrix;

struct Vector2
{
  Vector2() : x(0), y(0)
  {

  }

  Vector2(float val) : x(val), y(val)
  {

  }

  Vector2(float vx, float vy) : x(vx), y(vy)
  {

  }

  /* Vector2(const Vector2& rhs) : x(rhs.x), y(rhs.y)
  {

  } */

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

  void Normalize()
  {
    *this /= this->Length();
  }

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


  Vector2 GetNormalized() const
  {
    return *this / this->Length();
  }

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

struct Vector2i
{
  Vector2i() : x(0), y(0) {}

  Vector2i(const Vector2& vec2f)
  {
    x = (int)vec2f.x;
    y = (int)vec2f.y;
  }

  int x, y;
};

struct Vector3
{
  Vector3() : x(0), y(0), z(0) 
  {

  }

  Vector3(float val) : x(val), y(val), z(val) 
  {

  }

  Vector3(float vx, float vy, float vz) : x(vx), y(vy), z(vz) 
  {

  }

  /*   Vector3(const Vector3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z)
  {

  } */

  Vector3(const Vector4& rhs);

  inline float Length() const
  {
    return sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
  }

  static inline Vector3 Cross(const Vector3& v1, const Vector3& v2)
  {
    return Vector3(v1.y * v2.z - v1.z * v2.y,
                   v1.z * v2.x - v1.x * v2.z,
                   v1.x * v2.y - v1.y * v2.x);
  }

  static inline float Dot(const Vector3& v1, const Vector3& v2)
  {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  void Normalize()
  {
    *this /= this->Length();
  }

  static inline void Normalize(Vector3 &vec)
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

  inline Vector3 GetNormalized() const
  {
    return *this / this->Length();
  }

  static inline Vector3 GetNormalized(const Vector3 &vec)
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
    return out << "[" << val.x << ", " << val.y << ", " << val.z << "]";
  }

  inline void Clamp(int min, int max)
  {
      x = mathClamp(x, min, max);
      y = mathClamp(y, min, max);
      z = mathClamp(z, min, max);
  }

  Vector3 GetOrthonormalBasis() const 
  {
    Vector3 maximizedValue = Vector3::ZeroVector;
    if(x < y && x < z)
    {
      maximizedValue = Vector3(1.f, y, z);
    }
    else if(y < x && y < z)
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

  static const Vector3 ZeroVector;

  float x, y, z;
};

struct Vector3i
{
  Vector3i() : x(0), y(0), z(0) {}

  Vector3i(const Vector3& rhs)
  {
    x = (int)rhs.x;
    y = (int)rhs.y;
    z = (int)rhs.z;
  }

  int x, y, z;
};

struct Vector4
{
  Vector4() : x(0.f), y(0.f), z(0.f), w(0.f)
  {

  }

  Vector4(float value) : x(value), y(value), z(value), w(value)
  {

  }

  Vector4(float xVal, float yVal, float zVal, float wVal) : x(xVal), y(yVal), z(zVal), w(wVal)
  {

  }

  /* Vector4(const Vector4 &rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
  {

  } */

  Vector4(const Vector3 &rhs, float value = 0) : x(rhs.x), y(rhs.y), z(rhs.z), w(value)
  {

  }

  Vector4 operator*(const Matrix &rhs) const;

  inline friend std::ostream& operator<<(std::ostream& out, const Vector4& val)
  {
    return out << "[" << val.x << ", " << val.y << ", " << val.z << ", " << val.w << "]";
  }

  float x, y, z, w;
};

class Math
{
public:
  //static float Determinant(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);

  static inline float Determinant(const Vector3& a, const Vector3& b, const Vector3& c)
  {
    return    a.x * ((b.y * c.z) - (c.y * b.z))
            + a.y * ((c.x * b.z) - (b.x * c.z))
            + a.z * ((b.x * c.y) - (c.x * b.y));
  }

  static inline float Determinant(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
  {

    return a.x * Determinant(Vector3(b.y, b.z, b.w), Vector3(c.y, c.z, c.w), Vector3(d.y, d.z, d.w))
         - a.y * Determinant(Vector3(b.x, b.z, b.w), Vector3(c.x, c.z, c.w), Vector3(d.x, d.z, d.w))
         + a.z * Determinant(Vector3(b.x, b.y, b.w), Vector3(c.x, c.y, c.w), Vector3(d.x, d.y, d.w))
         - a.w * Determinant(Vector3(b.x, b.y, b.z), Vector3(c.x, c.y, c.z), Vector3(d.x, d.y, d.z));
  }
};

extern float EPSILON;
extern float INTERSECTION_TEST_EPSILON;
extern float SHADOW_EPSILON;

#endif