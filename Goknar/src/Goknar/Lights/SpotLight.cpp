/*
 *	Advanced ray-tracer algorithm
 *	Emre Baris Coskun
 *	2018
 */

#include "SpotLight.h"

Vector3 SpotLight::GetIntensityAtPosition(const Vector3& lightPosition, const Vector3& positionAt) const 
{
    Vector3 intersectionDirection = positionAt - position;
    float intersectionDirectionLength = intersectionDirection.Length();
    intersectionDirection.Normalize();

    // Angle between intersection direction and spot light's direction
    float cosTetha = Vector3::Dot(intersectionDirection, direction);

    if(cosTetha > cosFalloff)
    {
        return intensity / (intersectionDirectionLength * intersectionDirectionLength);
    }
    else if(cosTetha < cosCoverage)
    {
        return Vector3::ZeroVector;
    }
    
    //else
    float c = pow((cosTetha - cosCoverage) / (cosFalloff - cosCoverage), 4);
    return (intensity * c) / (intersectionDirectionLength * intersectionDirectionLength);
}