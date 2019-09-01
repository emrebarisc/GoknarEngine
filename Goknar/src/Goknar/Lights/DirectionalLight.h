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

    Vector3 GetDirection() const
    {
        return direction_;
    }

	void SetDirection(const Vector3& direction)
	{
		direction_ = direction;
	}
	
private:
    Vector3 direction_;
    Vector3 radiance_;
};

#endif