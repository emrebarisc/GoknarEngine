#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "Light.h"

#include "Math/Matrix.h"

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

	inline void UpdateBiasedShadowMatrix()
	{
		//biasedShadowViewProjectionMatrix_ =
		//	Matrix
		//{
		//	0.5f, 0.f, 0.f, 0.5f,
		//	0.f, 0.5f, 0.f, 0.5f,
		//	0.f, 0.f, 0.5f, 0.5f,
		//	0.f, 0.f, 0.f, 1.f
		//} * 
		//	shadowMapRenderCamera_->GetProjectionMatrix() *
		//	Matrix
		//{
		//	1.f, 0.f, 0.f, 0.f,
		//		0.f, 1.f, 0.f, 0.f,
		//		0.f, 0.f, 1.f, 0.05f,
		//		0.f, 0.f, 0.f, 1.f
		//} * 
		//	shadowMapRenderCamera_->GetViewMatrix();

		biasedShadowViewProjectionMatrix_ = shadowBiasMatrix_ * shadowMapRenderCamera_->GetViewProjectionMatrix();
	}

	void SetDirection(const Vector3& direction);

	virtual void SetIsShadowEnabled(bool isShadowEnabled) override;

	void RenderShadowMap() override;
	
private:
	Matrix biasedShadowViewProjectionMatrix_{ Matrix::IdentityMatrix };
	Matrix shadowBiasMatrix_{ Matrix::IdentityMatrix };

	Vector3 direction_{ Vector3::ForwardVector };
};

#endif