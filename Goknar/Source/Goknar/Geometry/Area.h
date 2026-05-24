#ifndef __AREA_H__
#define __AREA_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

class GOKNAR_API Area
{
public:
	Area();

	Vector3 GetCenter() const
	{
		return (point0 + point1 + point2 + point3) * 0.25f;
	}

	Vector3 point0{ Vector3::ZeroVector };
	Vector3 point1{ Vector3::ZeroVector };
	Vector3 point2{ Vector3::ZeroVector };
	Vector3 point3{ Vector3::ZeroVector };
};

#endif