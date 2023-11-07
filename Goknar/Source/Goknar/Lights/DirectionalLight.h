#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "Math/Matrix.h"
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

	void SetShaderUniforms(const Shader* shader) override;
	void SetShadowRenderPassShaderUniforms(const Shader* shader) override;

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
	Matrix biasedShadowViewProjectionMatrix_{ Matrix::IdentityMatrix };

    Vector3 direction_;
    Vector3 radiance_;
};

#endif