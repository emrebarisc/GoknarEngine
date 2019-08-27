/*
 *	Advanced ray-tracer algorithm
 *	Emre Baris Coskun
 *	2018
 */

#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Light.h"

#include "Math.h"

class SpotLight : public Light
{
public:
    SpotLight() : Light()
    {

    }

    // Pass angles in degrees
    SpotLight(float coverage, float falloff) : Light(), coverageAngle(coverage), falloffAngle(falloff)
    {
        coverageAngle = DEGREE_TO_RADIAN(coverageAngle);
        falloffAngle = DEGREE_TO_RADIAN(falloffAngle);
        cosFalloff = cos(falloffAngle);
        cosCoverage = cos(coverageAngle);
    }

    ~SpotLight() override
    {
        
    }

    Vector3 GetIntensityAtPosition(const Vector3& lightPosition, const Vector3& positionAt) const override;

    Vector3 direction;

    // Angles in radian. They are half of parsed values.
    float coverageAngle;
    float falloffAngle;

    // 100% light area
    float cosFalloff;

    // Transition area
    float cosCoverage;
private:
};

#endif