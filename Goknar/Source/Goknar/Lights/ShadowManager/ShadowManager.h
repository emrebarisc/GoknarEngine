#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

#include "Renderer/Types.h"
#include "Math/GoknarMath.h"

class IMaterialBase;
class Light;
class Shader;

#define DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX 0
#define POINT_LIGHT_UNIFORM_BIND_INDEX 1
#define SPOT_LIGHT_UNIFORM_BIND_INDEX 2

constexpr unsigned int MAX_DIRECTIONAL_LIGHT_COUNT = 4;
constexpr unsigned int MAX_POINT_LIGHT_COUNT = 16;
constexpr unsigned int MAX_SPOT_LIGHT_COUNT = 8;

class ShadowManager
{
public:
	ShadowManager();
	~ShadowManager();

	void PreInit();
	void Init();
	void PostInit();

	void RenderShadowMaps();
	void SetShadowRenderPassShaderUniforms(const Shader* shader) const;

	void BindLightUniforms(Shader* shader);

protected:

private:
	class DirectionalLightBufferInfo
	{
	public:
		class DirectionalLightInfo
		{
		public:
			Vector3 direction{ -1.f, -1.f, -1.f };
			int isCastingShadow{ false };
			Vector3 intensity{ 0.f, 0.f, 0.f };
			float padding;
		};
		DirectionalLightInfo directionalLightInfo[MAX_DIRECTIONAL_LIGHT_COUNT];
		int directionalLightCount;
	} directionalLightBufferInfo;

	Light* currentlyRenderingLight_;

	GEuint directionalLightUniformBufferId_;
	GEuint pointLightUniformBufferId_;
	GEuint spotLightUniformBufferId_;
};

#endif