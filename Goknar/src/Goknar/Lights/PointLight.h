#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

#include "Light.h"

class GOKNAR_API PointLight : public Light
{
public:
    PointLight() : Light()
    {
		id_ = currentId_;
		currentId_++;

		name_ = "PointLight" + id_;
    }

    ~PointLight() override
    {
        
    }

protected:

private:
	static int currentId_;
};

#endif