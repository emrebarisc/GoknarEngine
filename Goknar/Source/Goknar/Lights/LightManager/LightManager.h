#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

#include "Renderer/Types.h"
#include "Goknar/Renderer/ShaderBindingPoints.h"
#include "Goknar/Renderer/ShaderStorageBuffer.h"
#include "Math/GoknarMath.h"

class IMaterialBase;
class Shader;

class Light;
class DirectionalLight;
class PointLight;
class SpotLight;

inline constexpr GEuint DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX = ShaderBindingPoints::UniformBuffer::DIRECTIONAL_LIGHT_SHADOW_MATRICES;
inline constexpr GEuint SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX = ShaderBindingPoints::UniformBuffer::SPOT_LIGHT_SHADOW_MATRICES;

inline constexpr unsigned int MAX_DIRECTIONAL_LIGHT_COUNT = 4;
inline constexpr unsigned int MAX_POINT_LIGHT_COUNT = 16;
inline constexpr unsigned int MAX_SPOT_LIGHT_COUNT = 8;

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

	void UpdateLights();
	void MarkLightsDirty();

protected:

private:
	struct alignas(16) DirectionalLightInfo
	{
		Vector3 direction{ -1.f, -1.f, -1.f };
		float shadowIntensity{ 0.1f };
		Vector3 intensity{ 1.f, 1.f, 1.f };
		int isCastingShadow{ false };
	};

	struct alignas(16) PointLightInfo
	{
		Vector3 position{ 0.f, 0.f, 0.f };
		float radius{ 0.f };
		Vector3 intensity{ 1.f, 1.f, 1.f };
		int isCastingShadow{ false };
		float shadowIntensity{ 0.1f };
		float padding[3]{ 0.f, 0.f, 0.f };
	};

	struct alignas(16) SpotLightInfo
	{
		Vector3 position{ Vector3::ZeroVector };
		float coverageAngle{ 0.f };
		Vector3 direction{ Vector3::ForwardVector };
		float falloffAngle{ 0.f };
		Vector3 intensity{ 1.f, 1.f, 1.f };
		float shadowIntensity{ 0.1f };
		int isCastingShadow{ false };
		float padding[3]{ 0.f, 0.f, 0.f };
	};

	struct alignas(16) LightBufferInfo
	{
		DirectionalLightInfo directionalLightInfo[MAX_DIRECTIONAL_LIGHT_COUNT];
		PointLightInfo pointLightInfo[MAX_POINT_LIGHT_COUNT];
		SpotLightInfo spotLightInfo[MAX_SPOT_LIGHT_COUNT];
	} lightBufferInfo;

	static_assert(sizeof(DirectionalLightInfo) == sizeof(float) * 8, "DirectionalLightInfo must match the GLSL std430 struct layout.");
	static_assert(sizeof(PointLightInfo) == sizeof(float) * 12, "PointLightInfo must match the GLSL std430 struct layout.");
	static_assert(sizeof(SpotLightInfo) == sizeof(float) * 16, "SpotLightInfo must match the GLSL std430 struct layout.");

	void CollectDirectionalLightData();
	void CollectPointLightData();
	void CollectSpotLightData();
	void UploadLightDataToGPU();
	void UploadLightDataIfDirty();

	Light* currentlyRenderingLight_{ nullptr };

	GEuint directionalLightViewMatrixUniformBufferId_{ 0 };

	GEuint spotLightViewMatrixUniformBufferId_{ 0 };

	ShaderStorageBuffer lightStorageBuffer_;
	int directionalLightCount_{ 0 };
	int pointLightCount_{ 0 };
	int spotLightCount_{ 0 };
	bool lightDataDirty_{ true };
	bool isInitialized_{ false };
};

#endif
