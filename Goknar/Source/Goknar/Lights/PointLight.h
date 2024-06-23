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

        radius_ = 25.f;
    }

    virtual ~PointLight() override;

    void PreInit() override;
    void Init() override;
    void PostInit() override;

    void RenderShadowMap() override;
    void SetShaderUniforms(const Shader* shader) override;
    void SetShadowRenderPassShaderUniforms(const Shader* shader) override;

    void SetRadius(float radius)
    {
        radius_ = radius;
    }

    float GetRadius() const
    {
        return radius_;
    }

    virtual void SetPosition(const Vector3& position) override;

    virtual void SetIsShadowEnabled(bool isShadowEnabled) override;

protected:

private:
    void AllocateViewMatrices();
    void DeallocateViewMatrices();
    void UpdateShadowViewProjectionMatrices();

    Matrix* viewMatrices_{ nullptr };
    float radius_{ 100.f };
};

#endif