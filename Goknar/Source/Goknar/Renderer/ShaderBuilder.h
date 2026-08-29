#ifndef __SHADERBUILDER_H__
#define __SHADERBUILDER_H__

#include "Goknar/Core.h"
#include "Goknar/Model/MeshGeometry.h"

#include "Renderer/Renderer.h"
#include "ShaderTypes.h"

class Engine;
class MaterialInitializationData;
class Shader;
class Texture;

class GOKNAR_API ShaderBuilder
{
	friend Engine;

public:
	static ShaderBuilder* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ShaderBuilder();
		}

		return instance_;
	}

	std::string ForwardRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ForwardRenderPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ForwardRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string GeometryBufferPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string GeometryBufferPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string GeometryBufferPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string ShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ShadowPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string PointShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string PointShadowPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string PointShadowPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string PointShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string CubemapRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string CubemapRenderPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string CubemapRenderPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string CubemapRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string ParticleRenderPass_GetBillboardVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ParticleRenderPass_GetStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ParticleRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string GPUFoliageRenderPass_GetMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader, RenderPassType renderPassType) const;
	std::string GPUFoliageRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader, RenderPassType renderPassType) const;

	std::string DeferredRenderPass_GetVertexShaderScript();
	std::string DeferredRenderPass_GetFragmentShaderScript();
protected:

private:
	struct FragmentShaderInitializationData
	{
		FragmentShaderInitializationData() = delete;
		FragmentShaderInitializationData(const std::string& outVariables, const std::string& outVariableAssignments) :
			outputVariables(outVariables),
			outputVariableAssignments(outVariableAssignments)
		{}

		MaterialInitializationData* materialInitializationData{ nullptr };
		const Shader* shader{ nullptr };
		const std::string& outputVariables;
		const std::string& outputVariableAssignments;
		RenderPassType renderPassType{ RenderPassType::Forward };
	};

	struct VertexShaderInitializationData
	{
		VertexShaderInitializationData() {}

		MaterialInitializationData* materialInitializationData{ nullptr };
		const Shader* shader{ nullptr };
		RenderPassType renderPassType{ RenderPassType::Forward };
		MeshType meshType{ MeshType::None };
		bool usesGPUFoliageInstanceBuffer{ false };
		std::string vertexColorExpression;
	};

	static ShaderBuilder* instance_;

	ShaderBuilder() = default;
	~ShaderBuilder();

	std::string General_FS_GetScript(const FragmentShaderInitializationData& fragmentShaderInitializationData) const;
	std::string General_VS_GetScript(const VertexShaderInitializationData& vertexShaderInitializationData) const;

	std::string FS_GetLightSpaceFragmentPositions(const FragmentShaderInitializationData& fragmentShaderInitializationData) const;
	std::string FS_GetDirectionalLightColorFunction() const;
	std::string FS_GetPointLightColorFunction() const;
	std::string FS_GetSpotLightColorFunction() const;
	std::string FS_GetDirectionalLightStruct() const;
	std::string FS_GetPointLightStruct() const;
	std::string FS_GetSpotLightStruct() const;
	std::string FS_GetLightArrayUniforms() const;
	std::string FS_GetShadowMapUniforms() const;
	std::string FS_GetReflectionProbeUniforms() const;
	std::string FS_InitializeBaseColor(MaterialInitializationData* initializationData, bool discardTransparent = false) const;
	std::string FS_InitializeEmissiveColor(MaterialInitializationData* initializationData) const;
	std::string FS_InitializeAmbientOcclusion(MaterialInitializationData* initializationData) const;
	std::string FS_InitializeMetallic(MaterialInitializationData* initializationData) const;
	std::string FS_InitializeRoughness(MaterialInitializationData* initializationData) const;
	std::string FS_InitializeSurfaceNormal(MaterialInitializationData* initializationData) const;
	std::string FS_GetUnlitColorExpression(bool includeReflectionProbe) const;
	std::string FS_GetUnlitCheck(bool includeReflectionProbe) const;
	std::string FS_GetPBRFunctions(bool includeReflectionProbe) const;

	std::string General_FS_GetMaterialVariables(const FragmentShaderInitializationData& fragmentShaderInitializationData) const;
	void ApplyTextureBackedMaterialDefaults(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string General_FS_GetShaderTextureUniforms(const MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string General_FS_GetDiffuseTextureSampling(const Texture* texture) const;
	std::string General_FS_GetScalarTextureSampling(const Texture* texture) const;
	std::string General_FS_GetNormalTextureSampling(const Texture* texture) const;
	std::string General_FS_GetEmissiveTextureSampling(const Texture* texture) const;
	void General_FS_GetORMTextureSampling(const Texture* texture, MaterialInitializationData* initializationData) const;
	std::string General_FS_GetDiffuseTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetScalarTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetNormalTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetEmissiveTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetTextureUVExpression(const Texture* texture) const;

	std::string FS_GetLightCalculationIterators(bool includeShadowing) const;

	std::string FS_GetOutputVariables() const;
	std::string FS_GetOutputVariableAssignments() const;
	std::string FS_GetUnlitOutputVariableAssignments() const;

	std::string GeometryBufferPass_GetOutputVariables() const;
	std::string GeometryBufferPass_GetOutputVariableAssignments(MaterialInitializationData* initializationData) const;

	std::string DeferredRenderPass_GetGBufferTextureUniforms() const;
	std::string DeferredRenderPass_GetGBufferVariables() const;
	std::string DeferredRenderPass_GetGBufferVariableAssignments() const;
	std::string DeferredRenderPass_GetUnlitCheck() const;

	std::string VS_GetMainLayouts() const;
	std::string VS_GetInstancedStaticMeshLayouts() const;
	std::string VS_GetSkeletalMeshLayouts() const;
	std::string VS_GetSkeletalMeshVariables() const;
	std::string VS_GetSkeletalMeshUniforms(int boneCount) const;
	std::string VS_GetInstancedStaticMeshTransformationMatrixCalculation() const;
	std::string VS_GetGPUFoliageInstanceBuffer() const;
	std::string VS_GetUniforms() const;
	std::string VS_GetLightShadowViewMatrixUniforms() const;
	std::string VS_GetLightOutputs() const;
	std::string VS_GetSkeletalMeshWeightCalculation() const;
	std::string VS_GetMain(const VertexShaderInitializationData& vertexShaderInitializationData, const std::string& vertexShaderModelMatrixVariable) const;
	std::string VS_GetPosition() const;
	std::string VS_GetWorldPositionOffsetText(MaterialInitializationData* initializationData, const std::string& worldPositionVariableName) const;
	std::string VS_GetUV(MaterialInitializationData* initializationData) const;
	std::string VS_GetLightSpaceFragmentPositionCalculations() const;
	std::string VS_GetVertexNormalText(MaterialInitializationData* initializationData) const;
	std::string VS_GetVertexTangentText(const std::string& tangentExpression = "", const std::string& tangentSignExpression = "") const;
	std::string VS_GetVertexColorText(const std::string& colorExpression = "") const;

	std::string shaderVersion_{ DEFAULT_SHADER_VERSION };
};

#endif
