#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "Light.h"

class DirectionalLight : public Light
{
public:
    DirectionalLight() : Light()
    {

    }

    ~DirectionalLight() override
    {
        
    }

    Vector3 GetDirection(const Vector3 &lightPosition, const Vector3 &referencePosition) const override
    {
        return direction;
    }

    Vector3 direction;
    Vector3 radiance;
private:
};

#endif