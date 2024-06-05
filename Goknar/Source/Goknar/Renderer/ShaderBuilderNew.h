#ifndef __SHADERBUILDERNEW_H__
#define __SHADERBUILDERNEW_H__

#include "Goknar/Core.h"

#include "ShaderTypes.h"

class MaterialInitializationData;
class Shader;

class GOKNAR_API ShaderBuilderNew
{
public:
	static ShaderBuilderNew* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ShaderBuilderNew();
		}

		return instance_;
	}

	std::string ForwardRender_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string ForwardRender_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const;

protected:

private:
	static ShaderBuilderNew* instance_;

	ShaderBuilderNew() = default;
	~ShaderBuilderNew();

	std::string FS_GetMaterialVariables() const;
	std::string FS_GetLightSpaceFragmentPositions() const;
	std::string FS_GetDirectionalLightColorFunction() const;
	std::string FS_GetPointLightColorFunction() const;
	std::string FS_GetSpotLightColorFunction() const;
	std::string FS_GetDirectionalLightStruct() const;
	std::string FS_GetPointLightStruct() const;
	std::string FS_GetSpotLightStruct() const;
	std::string FS_GetLightArrayUniforms() const;
	std::string FS_GetShadowMapUniforms() const;

	std::string FS_GetShaderTextureUniforms(MaterialInitializationData* initializationData, const Shader* shader) const;
	std::string FS_GetDiffuseTextureSampling(const std::string& textureName) const;
	std::string FS_GetNormalTextureSampling(const std::string& textureName) const;

	std::string FS_GetLightCalculationIterators() const;
	std::string FS_InitializeBaseColor(MaterialInitializationData* initializationData) const;
	std::string FS_GetFinalColorCalculation() const;

	std::string VS_GetMainLayouts() const;
	std::string VS_GetSkeletalMeshLayouts() const;
	std::string VS_GetSkeletalMeshVariables() const;
	std::string VS_GetSkeletalMeshUniforms(int boneCount) const;
	std::string VS_GetUniforms() const;
	std::string VS_GetLightShadowViewMatrixUniforms() const;
	std::string VS_GetLightOutputs() const;
	std::string VS_GetSkeletalMeshWeightCalculation() const;
	std::string VS_GetMain(const std::string& vertexShaderModelMatrixVariable) const;
	std::string VS_GetLightSpaceFragmentPositionCalculations() const;
	std::string VS_GetVertexNormalText() const;

	std::string shaderVersion_{ DEFAULT_SHADER_VERSION };
};

#endif