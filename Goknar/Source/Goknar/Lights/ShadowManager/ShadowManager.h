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

#define DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX 3
#define SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX 4

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

	void BindShadowViewProjectionMatrices();

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
		int directionalLightCount{ 0 };
	} directionalLightBufferInfo;

	class PointLightBufferInfo
	{
	public:
		class PointLightInfo
		{
		public:

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

		};
		SpotLightInfo spotLightInfo[MAX_SPOT_LIGHT_COUNT];
		int spotLightCount{ 0 };
	} spotLightBufferInfo;

	Light* currentlyRenderingLight_;

	GEuint directionalLightUniformBufferId_;
	GEuint directionalLightViewMatrixUniformBufferId_;

	GEuint pointLightUniformBufferId_;
	GEuint pointLightViewMatrixUniformBufferId_;

	GEuint spotLightUniformBufferId_;
	GEuint spotLightViewMatrixUniformBufferId_;
};

#endif