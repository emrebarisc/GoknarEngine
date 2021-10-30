#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "Light.h"

#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

class GOKNAR_API DirectionalLight : public Light
{
public:
	DirectionalLight();
	virtual ~DirectionalLight() override
    {
        
    }

	void Init() override;

	void SetShaderUniforms(const Shader* shader) const;

    Vector3 GetDirection() const
    {
        return direction_;
    }

	void SetDirection(const Vector3& direction)
	{
		direction_ = direction.GetNormalized();
	}

	void RenderShadowMap() override;
	
private:
    Vector3 direction_;
    Vector3 radiance_;
};

#endif