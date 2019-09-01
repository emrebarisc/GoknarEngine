#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Light.h"

#include "Math.h"

class GOKNAR_API SpotLight : public Light
{
public:
    SpotLight() : Light()
    {

    }

    // Pass angles in degrees
    SpotLight(float coverage, float falloff) : Light(), coverageAngle_(coverage), falloffAngle_(falloff)
    {
        coverageAngle_ = DEGREE_TO_RADIAN(coverageAngle_);
        falloffAngle_ = DEGREE_TO_RADIAN(falloffAngle_);
        cosFalloff_ = cos(falloffAngle_);
        cosCoverage_ = cos(coverageAngle_);
    }

    ~SpotLight() override
    {
        
    }

	const Vector3& GetDirection() const
	{
		return direction_;
	}

	void SetDirection(const Vector3& direction)
	{
		direction_ = direction;
	}

	float GetCoverageAngle() const
	{
		return coverageAngle_;
	}

	void SetCoverageAngle(float coverageAngle)
	{
		coverageAngle_ = coverageAngle;
	}

	float GetFalloffAngle() const
	{
		return falloffAngle_;
	}

	void SetFalloffAngle(float coverageAngle)
	{
		coverageAngle_ = coverageAngle;
	}

private:
    Vector3 direction_;

    // Angles in radian. They are half of parsed values.
    float coverageAngle_;
    float falloffAngle_;

    // 100% light area
    float cosFalloff_;

    // Transition area
    float cosCoverage_;
private:
};

#endif