#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Light.h"

#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

#include "Math.h"

class GOKNAR_API SpotLight : public Light
{
public:
    SpotLight() : Light(), coverageAngle_(0.f), falloffAngle_(0.f)
    {
		id_ = ObjectIDManager::GetInstance()->GetAndIncreaseSpotLightID();
		name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT) + std::to_string(id_);
    }

    // Pass angles in degrees
    SpotLight(float coverage, float falloff) : Light(), coverageAngle_(coverage), falloffAngle_(falloff)
    {
        coverageAngle_ = DEGREE_TO_RADIAN(coverageAngle_);
        falloffAngle_ = DEGREE_TO_RADIAN(falloffAngle_);
    }

    ~SpotLight() override
    {
        
    }

	void SetShaderUniforms(const Shader* shader) const;

	const Vector3& GetDirection() const
	{
		return direction_;
	}

	void SetDirection(const Vector3& direction)
	{
		direction_ = direction.GetNormalized();
	}

	float GetCoverageAngle() const
	{
		return coverageAngle_;
	}

	void SetCoverageAngle(float coverageAngleInDegrees)
	{
		coverageAngle_ = DEGREE_TO_RADIAN(coverageAngleInDegrees);
	}

	float GetFalloffAngle() const
	{
		return falloffAngle_;
	}

	void SetFalloffAngle(float falloffAngleInDegrees)
	{
		falloffAngle_ = DEGREE_TO_RADIAN(falloffAngleInDegrees);
	}

private:
    Vector3 direction_;

    // Angles in radian. They are half of parsed values.
    float coverageAngle_;
    float falloffAngle_;
private:

	static int lastSpotLightId_;
};

#endif