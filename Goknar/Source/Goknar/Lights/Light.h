#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Renderer/Types.h"

class Camera;
class Framebuffer;
class Shader;
class Texture;

enum class LightMobility : uint8_t
{
	Static = 0,
	Dynamic
};

#define POINT_LIGHT_TYPE_INT 0
#define DIRECTIONAL_LIGHT_TYPE_INT 1
#define SPOT_LIGHT_TYPE_INT 2

class GOKNAR_API Light
{
public:
	Light() = default;

	virtual ~Light();

	virtual void Init();

	virtual void SetPosition(const Vector3& position)
	{
		position_ = position;
	}

	const Vector3& GetPosition() const
    {
        return position_;
    }

	void SetColor(const Vector3& color)
	{
		color_ = color;
	}

	const Vector3& GetColor() const
	{
		return color_;
	}

	void SetIntensity(float intensity)
	{
		intensity_ = intensity;
	}

	float GetIntensity() const
	{
		return intensity_;
	}

	LightMobility GetLightMobility() const
	{
		return mobility_;
	}

	void SetLightMobility(LightMobility mobility)
	{
		mobility_ = mobility;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}
	
	int GetID() const
	{
		return id_;
	}

	void SetID(int id)
	{
		id_ = id;
	}

	bool GetIsShadowEnabled() const
	{
		return isShadowEnabled_;
	}

	virtual void SetIsShadowEnabled(bool isShadowEnabled)
	{
		isShadowEnabled_ = isShadowEnabled;
	}

	int GetShadowWidth()
	{
		return shadowWidth_;
	}

	int GetShadowHeight()
	{
		return shadowHeight_;
	}

	Texture* GetShadowMapTexture()
	{
		return shadowMapTexture_;
	}

	Camera* GetShadowMapRenderCamera()
	{
		return shadowMapRenderCamera_;
	}

	Framebuffer* GetShadowMapFramebuffer()
	{
		return shadowMapFramebuffer_;
	}

	virtual void SetShaderUniforms(const Shader* shader);

	virtual void RenderShadowMap() = 0;
	virtual void SetShadowRenderPassShaderUniforms(const Shader* shader) = 0;

protected:

	Vector3 position_{ Vector3::ZeroVector };
	Vector3 color_{ Vector3::ZeroVector };

	Camera* shadowMapRenderCamera_{ nullptr };
	Framebuffer* shadowMapFramebuffer_{ nullptr };
	Texture* shadowMapTexture_{ nullptr };

	std::string name_{ "" };

	int id_{ 0 };

	int shadowWidth_{ 1024 * 2 };
	int shadowHeight_{ 1024 * 2 };

	float intensity_{ 1.f };

	bool isShadowEnabled_{ false };

	LightMobility mobility_{ LightMobility::Static };
private:
};

#endif