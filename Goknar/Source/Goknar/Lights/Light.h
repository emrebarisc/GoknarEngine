#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Goknar/GoknarMath.h"
#include "Goknar/Renderer/Types.h"

class Camera;
class Shader;
class Texture;

enum class LightMobility : uint8_t
{
	Static = 0,
	Movable
};

class GOKNAR_API Light
{
public:
	Light();

	virtual ~Light();

	virtual void SetShaderUniforms(const Shader* shader) const;

	virtual void Init();

	void SetPosition(const Vector3& position)
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

	void SetIsShadowEnabled(bool isShadowEnabled)
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

	GEuint GetShadowMapFBO()
	{
		return shadowMapDepthFBO_;
	}

	virtual void RenderShadowMap() = 0;
	Texture* GetShadowMapTexture()
	{
		return shadowMapTexture_;
	}

	Camera* GetShadowMapRenderCamera()
	{
		return shadowMapRenderCamera_;
	}

protected:

	Vector3 position_;
	Vector3 color_;

	Texture* shadowMapTexture_;
	Camera* shadowMapRenderCamera_;

	std::string name_;

	int id_;

	int shadowWidth_;
	int shadowHeight_;

	float intensity_;

	GEuint shadowMapDepthFBO_;
	bool isShadowEnabled_;

private:
	LightMobility mobility_;
};

#endif