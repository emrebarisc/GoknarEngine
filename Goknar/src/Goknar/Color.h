#ifndef __COLOR_H__
#define __COLOR_H__

#include <iostream>

#include "Core.h"
#include "Math.h"

class GOKNAR_API Colori
{
public:
    Colori()
        : r(0), g(0), b(0)
    {

    }

    Colori(const Vector3 &vector) 
    : r((int)vector.x), g((int)vector.y), b((int)vector.z)
    {}

    Colori(const Vector3i &vector) 
    : r(vector.x), g(vector.y), b(vector.z)
    {}

    Colori(const volatile Vector3i &vector) 
    : r(vector.x), g(vector.y), b(vector.z)
    {}

    Colori(int red, int green, int blue) 
    : r(red), g(green), b(blue)
    {}

    ~Colori()
    {
        
    }

    inline void operator+=(const Colori& rhs)
    {
        r = r + rhs.r;
        g = g + rhs.g;
        b = b + rhs.b;
    }

    inline void operator/=(float val)
    {
        r = (int)(r / val);
		g = (int)(g / val);
		b = (int)(b / val);
    }

    inline friend Colori operator*(float val, const Colori &color)
    {
        return Colori((int)(color.r * val), (int)(color.g * val), (int)(color.b * val));
    }

    inline Colori operator*(float val) const
    {
        return Colori((int)(r * val), (int)(g * val), (int)(b * val));
    }

    inline Colori operator*(int val) const
    {
        return Colori(r * val, g * val, b * val);
    }

    inline Colori operator/(float val) const
    {
        return Colori((int)(r / val), (int)(g / val), (int)(b / val));
    }

    inline Colori operator/(int val) const
    {
        return Colori(r / val, g / val, b / val);
    }

    inline void ClampColor(int min, int max)
    {
        r = mathClamp(r, min, max);
        g = mathClamp(g, min, max);
        b = mathClamp(b, min, max);
    }

    int r, g, b;
private:

};

class Colorf
{
public:
    Colorf()
        : r(0.f), g(0.f), b(0.f)
    {

    }

    Colorf(float value) 
    : r(value), g(value), b(value)
    {}

    Colorf(const Vector3 &vector) 
    : r(vector.x), g(vector.y), b(vector.z)
    {}

    Colorf(const Vector3i &vector) 
    : r((float)vector.x), g((float)vector.y), b((float)vector.z)
    {}

    Colorf(float red, float green, float blue) 
    : r(red), g(green), b(blue)
    {}

    ~Colorf()
    {

    }

    inline void ClampColor(float min, float max)
    {
        r = mathClamp(r, min, max);
        g = mathClamp(g, min, max);
        b = mathClamp(b, min, max);
    }

    inline Colorf operator-() const
    {
        return Colorf(-r, -g, -b);
    }

    inline Colorf operator-(const Colorf& rhs) const
    {
        return Colorf(r - rhs.r, g - rhs.g, b - rhs.b);
    }

    inline Colorf operator+(const Colorf& rhs) const
    {
        return Colorf(r + rhs.r, g + rhs.g, b + rhs.b);
    }

    inline void operator=(const Colorf& rhs)
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
    }

    inline void operator=(const Colori& rhs)
    {
        r = (float)rhs.r;
        g = (float)rhs.g;
        b = (float)rhs.b;
    }

    inline void operator+=(const Colorf& rhs)
    {
        r = r + rhs.r;
        g = g + rhs.g;
        b = b + rhs.b;
    }

    inline Colorf operator*(float val) const
    {
        return Colorf(r * val, g * val, b * val);
    }

    inline friend Colorf operator*(float val, const Colorf& rhs)
    {
        return Colorf(rhs.r * val, rhs.g * val, rhs.b * val);
    }

    inline Colorf operator*(const Colorf& rhs) const
    {
        return Colorf(r * rhs.r, g * rhs.g, b * rhs.b);
    }

    inline Colorf operator/(float val) const
    {
        return Colorf(r / val, g / val, b / val);
    }

    inline Colorf operator/(const Colorf& rhs) const
    {
        return Colorf(r / rhs.r, g / rhs.g, b / rhs.b);
    }

    inline void operator/=(const float& val)
    {
        r /= val;
        g /= val;
        b /= val;
    }

    inline bool operator==(const Colorf& val) const
    {
        return r == val.r && g == val.g && b == val.b;
    }

    inline bool operator!=(const Colorf& val) const
    {
        return r != val.r && g != val.g && b != val.b;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const Colorf& val)
    {
        return out << "[" << val.r << ", " << val.g << ", " << val.b << "]";
    }


    float r, g, b;
private:

};

#endif