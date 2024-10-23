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
	virtual ~DirectionalLight() override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;

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

	const Matrix& GetBiasedShadowViewProjectionMatrix() const
	{
		return biasedShadowViewProjectionMatrix_;
	}

	void SetDirection(const Vector3& direction);

	virtual void SetIsShadowEnabled(bool isShadowEnabled) override;

	void RenderShadowMap() override;

	float GetShadowBiasValue() const
	{
		return shadowBiasValue_;
	}

	void SetShadowBiasValue(float biasValue)
	{
		shadowBiasValue_ = biasValue;
	}
	
private:
	Matrix biasedShadowViewProjectionMatrix_{ Matrix::IdentityMatrix };
	Matrix shadowBiasMatrix_{ Matrix::IdentityMatrix };

	Vector3 direction_{ Vector3::ForwardVector };

	float shadowBiasValue_{ 0.0001f };
};

#endif