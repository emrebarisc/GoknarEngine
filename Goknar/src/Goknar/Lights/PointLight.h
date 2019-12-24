#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

#include "Light.h"

#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

class GOKNAR_API PointLight : public Light
{
public:
    PointLight() : Light()
    {
		id_ = lastPointLightId_++;
		name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT) + std::to_string(id_);
    }

    ~PointLight() override
    {
        
    }

protected:

private:
	static int lastPointLightId_;
};

#endif