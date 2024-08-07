#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/Camera.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Renderer/Types.h"

class FrameBuffer;
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

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

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

	void SetShadowIntensity(float shadowIntensity)
	{
		shadowIntensity_ = shadowIntensity;
	}

	float GetShadowIntensity() const
	{
		return shadowIntensity_;
	}

	LightMobility GetLightMobility() const
	{
		return mobility_;
	}

	void SetLightMobility(LightMobility mobility)
	{
		mobility_ = mobility;
	}

	std::string GetName() const
	{
		return std::to_string(GUID_) + "_" + name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}
	
	int GetGUID() const
	{
		return GUID_;
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

	void SetShadowWidth(int shadowWidth)
	{
		shadowWidth_ = shadowWidth;
	}

	int GetShadowHeight()
	{
		return shadowHeight_;
	}

	void SetShadowHeight(int shadowHeight)
	{
		shadowHeight_ = shadowHeight;
	}

	Texture* GetShadowMapTexture()
	{
		return shadowMapTexture_;
	}

	Camera* GetShadowMapRenderCamera()
	{
		return shadowMapRenderCamera_;
	}

	FrameBuffer* GetShadowMapFrameBuffer()
	{
		return shadowMapFrameBuffer_;
	}

	int GetUniformBufferIndex() const
	{
		return uniformBufferIndex_;
	}

	void SetUniformBufferIndex(int uniformBufferIndex)
	{
		uniformBufferIndex_ = uniformBufferIndex;
	}

	virtual void SetShaderUniforms(const Shader* shader);

	virtual void RenderShadowMap() = 0;
	virtual void SetShadowRenderPassShaderUniforms(const Shader* shader) = 0;

protected:
	Vector3 position_{ Vector3::ZeroVector };
	Vector3 color_{ Vector3{ 1.f } };

	Camera* shadowMapRenderCamera_{ nullptr };
	FrameBuffer* shadowMapFrameBuffer_{ nullptr };
	Texture* shadowMapTexture_{ nullptr };

	std::string name_{ "" };

	int GUID_{ 0 };
	int uniformBufferIndex_{ -1 };

	int shadowWidth_{ 1024 };
	int shadowHeight_{ 1024 };

	float intensity_{ 1.f };
	float shadowIntensity_{ 0.1f };

	bool isShadowEnabled_{ false };

	LightMobility mobility_{ LightMobility::Static };
private:
};

#endif