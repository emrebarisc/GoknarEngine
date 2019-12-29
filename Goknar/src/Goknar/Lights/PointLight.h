#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

#include "Light.h"

#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

class GOKNAR_API PointLight : public Light
{
public:
    PointLight() : Light()
    {
		id_ = ObjectIDManager::GetInstance()->GetAndIncreasePointLightID();
		name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT) + std::to_string(id_);
    }

    ~PointLight() override
    {
        
    }

protected:

private:
};

#endif