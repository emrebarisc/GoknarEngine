#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

#include "Renderer/Types.h"
#include "Math/GoknarMath.h"

class IMaterialBase;
class Shader;

class Light;
class DirectionalLight;
class PointLight;
class SpotLight;

#define DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX 0
#define POINT_LIGHT_UNIFORM_BIND_INDEX 1
#define SPOT_LIGHT_UNIFORM_BIND_INDEX 2

#define DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX 3
#define SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX 4

constexpr unsigned int MAX_DIRECTIONAL_LIGHT_COUNT = 4;
constexpr unsigned int MAX_POINT_LIGHT_COUNT = 16;
constexpr unsigned int MAX_SPOT_LIGHT_COUNT = 8;

class LightManager
{
public:
	LightManager();
	~LightManager();

	void PreInit();
	void Init();
	void PostInit();

	void RenderShadowMaps();
	void SetShadowRenderPassShaderUniforms(const Shader* shader) const;

	void BindLightUniforms(Shader* shader);

	void BindShadowViewProjectionMatrices();

	void OnDirectionalLightAdded(DirectionalLight* directionalLight);
	void OnDirectionalLightRemoved(DirectionalLight* directionalLight);
	void OnPointLightAdded(PointLight* pointLight);
	void OnPointLightRemoved(PointLight* pointLight);
	void OnSpotLightAdded(SpotLight* spotLight);
	void OnSpotLightRemoved(SpotLight* spotLight);

protected:

private:
	class DirectionalLightBufferInfo
	{
	public:
		class DirectionalLightInfo
		{
		public:
			Vector3 direction{ -1.f, -1.f, -1.f };
			float shadowIntensity{ 0.1f };
			Vector3 intensity{ 1.f, 1.f, 1.f };
			int isCastingShadow{ false };
		};
		DirectionalLightInfo directionalLightInfo[MAX_DIRECTIONAL_LIGHT_COUNT];
		int directionalLightCount{ 0 };
	} directionalLightBufferInfo;

	class PointLightBufferInfo
	{
	public:
		class PointLightInfo
		{
		public:
			Vector3 position{ 0.f, 0.f, 0.f};
			float radius{ 0.f };
			Vector3 intensity{ 1.f, 1.f, 1.f };
			int isCastingShadow{ false };
			float shadowIntensity{ 0.1f };
			Vector3 padding{ Vector3::ZeroVector };
		};
		PointLightInfo pointLightInfo[MAX_POINT_LIGHT_COUNT];
		int pointLightCount{ 0 };
	} pointLightBufferInfo;

	class SpotLightBufferInfo
	{
	public:
		class SpotLightInfo
		{
		public:
			Vector3 position;
			float coverageAngle;
			Vector3 direction;
			float falloffAngle;
			Vector3 intensity;
			float shadowIntensity{ 0.1f };
			int isCastingShadow;
			Vector3 padding{ Vector3::ZeroVector };
		};
		SpotLightInfo spotLightInfo[MAX_SPOT_LIGHT_COUNT];
		int spotLightCount{ 0 };
	} spotLightBufferInfo;

	void UpdateAllDirectionalLightDataOnGPU();
	void UpdateAllPointLightDataOnGPU();
	void UpdateAllSpotLightDataOnGPU();

	Light* currentlyRenderingLight_{ nullptr };

	GEuint directionalLightUniformBufferId_{ 0 };
	GEuint directionalLightViewMatrixUniformBufferId_{ 0 };

	GEuint pointLightUniformBufferId_{ 0 };

	GEuint spotLightUniformBufferId_{ 0 };
	GEuint spotLightViewMatrixUniformBufferId_{ 0 };

	bool isInitialized_{ false };
};

#endif