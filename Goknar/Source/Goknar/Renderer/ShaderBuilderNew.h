#ifndef __SHADERBUILDERNEW_H__
#define __SHADERBUILDERNEW_H__

#include "Goknar/Core.h"

#include "Renderer/Renderer.h"
#include "ShaderTypes.h"

class Engine;
class MaterialInitializationData;
class Shader;

class GOKNAR_API ShaderBuilderNew
{
	friend Engine;

public:
	static ShaderBuilderNew* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ShaderBuilderNew();
		}

		return instance_;
	}

	std::string ForwardRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ForwardRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

	std::string GeometryBufferPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string GeometryBufferPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	
	std::string ShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	
	std::string PointShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string PointShadowPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string PointShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

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
	};

	static ShaderBuilderNew* instance_;

	ShaderBuilderNew() = default;
	~ShaderBuilderNew();

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
	std::string FS_InitializeBaseColor(MaterialInitializationData* initializationData) const;
	std::string FS_InitializeEmmisiveColor(MaterialInitializationData* initializationData) const;

	std::string General_FS_GetMaterialVariables(const FragmentShaderInitializationData& fragmentShaderInitializationData) const;
	std::string General_FS_GetShaderTextureUniforms(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string General_FS_GetDiffuseTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetNormalTextureSampling(const std::string& textureName) const;
	std::string General_FS_GetEmmisiveTextureSampling(const std::string& textureName) const;

	std::string FS_GetLightCalculationIterators() const;

	std::string FS_GetOutputVariables() const;
	std::string FS_GetOutputVariableAssignments() const;

	std::string GeometryBufferPass_GetOutputVariables() const;
	std::string GeometryBufferPass_GetOutputVariableAssignments() const;

	std::string DeferredRenderPass_GetGBufferTextureUniforms() const;
	std::string DeferredRenderPass_GetGBufferVariables() const;
	std::string DeferredRenderPass_GetGBufferVariableAssignments() const;

	std::string VS_GetMainLayouts() const;
	std::string VS_GetSkeletalMeshLayouts() const;
	std::string VS_GetSkeletalMeshVariables() const;
	std::string VS_GetSkeletalMeshUniforms(int boneCount) const;
	std::string VS_GetUniforms() const;
	std::string VS_GetLightShadowViewMatrixUniforms() const;
	std::string VS_GetLightOutputs() const;
	std::string VS_GetSkeletalMeshWeightCalculation() const;
	std::string VS_GetMain(const VertexShaderInitializationData& vertexShaderInitializationData, const std::string& vertexShaderModelMatrixVariable) const;
	std::string VS_GetPosition() const;
	std::string VS_GetPositionOffset(MaterialInitializationData* initializationData) const;
	std::string VS_GetUV(MaterialInitializationData* initializationData) const;
	std::string VS_GetLightSpaceFragmentPositionCalculations() const;
	std::string VS_GetVertexNormalText(MaterialInitializationData* initializationData) const;
	std::string VS_GetVertexColorText() const;

	std::string shaderVersion_{ DEFAULT_SHADER_VERSION };
};

#endif