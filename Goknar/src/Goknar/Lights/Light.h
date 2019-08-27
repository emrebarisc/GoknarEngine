#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/Math.h"

class Light
{
public:
    Light()
    {
        
    }

    virtual ~Light()
    {
        
    }

    virtual Vector3 GetPosition() const
    {
        return position;
    }

    virtual Vector3 GetDirection(const Vector3 &lightPosition, const Vector3 &referencePosition) const
    {
        return Vector3::GetNormalized(referencePosition - lightPosition);
    }
	
    Vector3 position;
    Vector3 intensity;
};

#endif