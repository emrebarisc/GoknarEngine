#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "Light.h"

class GOKNAR_API DirectionalLight : public Light
{
public:
	DirectionalLight() : Light() 
	{
		id_ = currentId_;
		currentId_++;

		name_ = "DirectionalLight" + id_;
	}

    ~DirectionalLight() override
    {
        
    }

	void SetShaderUniforms(const Shader* shader);

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

	static int currentId_;
};

#endif