#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Light.h"

#include "Math/Matrix.h"

#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

class GOKNAR_API SpotLight : public Light
{
public:
	SpotLight();
	SpotLight(float coverage, float falloff);
	virtual ~SpotLight() override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;

	void SetShaderUniforms(const Shader* shader) override;
	void SetShadowRenderPassShaderUniforms(const Shader* shader) override;

	void SetPosition(const Vector3& position) override;

	const Vector3& GetDirection() const
	{
		return direction_;
	}

	void SetDirection(const Vector3& direction);

	float GetCoverageAngle() const
	{
		return coverageAngle_;
	}

	void SetCoverageAngle(float coverageAngleInDegrees);

	float GetFalloffAngle() const
	{
		return falloffAngle_;
	}

	void SetFalloffAngle(float falloffAngleInDegrees)
	{
		falloffAngle_ = DEGREE_TO_RADIAN(falloffAngleInDegrees);
	}

	void SetRadius(float radius)
	{
		radius_ = radius;
	}

	float GetRadius() const
	{
		return radius_;
	}

	inline void UpdateBiasedShadowMatrix()
	{
		biasedShadowViewProjectionMatrix_ = shadowBiasMatrix_ * shadowMapRenderCamera_->GetViewProjectionMatrix();
	}

	const Matrix& GetBiasedShadowViewProjectionMatrix() const
	{
		return biasedShadowViewProjectionMatrix_;
	}

	virtual void SetIsShadowEnabled(bool isShadowEnabled) override;

	void RenderShadowMap() override;

private:
	Matrix biasedShadowViewProjectionMatrix_{ Matrix::IdentityMatrix };
	Matrix shadowBiasMatrix_{ Matrix::IdentityMatrix };

	Vector3 direction_{ Vector3::ForwardVector };

    // Angles in radian. They are half of parsed values.
    float coverageAngle_;
    float falloffAngle_;

	float radius_{ 100.f };
private:

	static int lastSpotLightId_;
};

#endif