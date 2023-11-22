#ifndef __SHADERBUILDER_H__
#define __SHADERBUILDER_H__

#include "Goknar/Core.h"

class DirectionalLight;
class Engine;
class Material;
class MeshUnit;
class PointLight;
class SpotLight;
class Shader;
class Scene;

class GOKNAR_API ShaderBuilder
{
public:
	friend Engine;

	static ShaderBuilder* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ShaderBuilder();
		}

		return instance_;
	}

	void Init();

	const std::string& GetDefaultSceneVertexShader() const
	{
		return sceneVertexShader_;
	}

	const std::string& GetDefaultSceneFragmentShader() const
	{
		return sceneFragmentShader_;
	}

	std::string GetShaderVersion() const
	{
		return shaderVersion_;
	}

	void SetShaderVersion(const std::string& shaderVersion)
	{
		shaderVersion_ = shaderVersion;
	}

	void BuildShader(MeshUnit* mesh, Material* material);
	std::string BuildVertexShader_ForwardRendering(MeshUnit* mesh);
	std::string BuildVertexShader_ShadowPass(MeshUnit* mesh);
	std::string BuildVertexShader_PointLightShadowPass(MeshUnit* mesh);

	std::string BuildFragmentShader_ForwardRendering(Material* material);

	bool GetIsInstatiated() const
	{
		return isInstantiated_;
	}

	std::string GetVertexShaderScript_DeferredPass();

	std::string GetFragmentShaderScript_ShadowPass();
	std::string GetFragmentShaderScript_PointLightShadowPass();
	std::string GetFragmentShaderScript_GeometryBufferPass(const Material* const material);
	std::string GetFragmentShaderScript_DeferredPass();

	static std::string GetGeometryShaderScript_PointLightShadowPass();

protected:

private:
	static ShaderBuilder* instance_;

	ShaderBuilder();
	~ShaderBuilder();

	void ResetVariables();
	
	std::string shaderVersion_;

	// Vertex shader
	std::string VS_BuildScene();
	std::string VS_GetMainLayouts();
	std::string VS_GetSkeletalMeshLayouts();
	std::string VS_GetSkeletalMeshUniforms(int boneCount);
	std::string VS_GetSkeletalMeshVariables();
	std::string VS_GetSkeletalMeshWeightCalculation();
	std::string VS_GetUniforms();
	std::string VS_GetLightOutputs();
	std::string VS_GetMain();
	std::string VS_GetMain_ShadowPass();
	std::string VS_GetMain_PointLightShadowPass();
	std::string VS_GetVertexNormalText();

	std::string vertexShaderOutsideMain_;
	std::string vertexShaderInsideMain_;
	std::string sceneVertexShader_;

	// Fragment Shader
	void FS_BuildSceneForwardRendering();
	void FS_BuildLightOperations(const Scene* scene);
	std::string FS_GetVariableTexts();

	// Fragment shader builder getter functions
	std::string GetShaderVersionText() const;
	std::string GetMaterialVariables();
	std::string GetMaterialDiffuseVariable();
	std::string GetTextureDiffuseVariable();

	void CombineFragmentShader();

	// Point Light
	std::string GetPointLightUniformTexts(const std::string& lightVariableName);
	std::string GetPointLightColorFunctionText();
	std::string GetStaticPointLightText(const PointLight* pointLight) const;
	std::string GetPointLightColorSummationText(const std::string& lightVariableName);

	// Directional Light
	std::string GetDirectionalLightUniformTexts(const std::string& lightVariableName);
	std::string GetDirectionalLightColorFunctionText();
	std::string GetStaticDirectionalLightText(const DirectionalLight* directionalLight) const;
	std::string GetDirectionalLightColorSummationText(const std::string& lightVariableName);

	// Spot Light
	std::string GetSpotLightUniformTexts(const std::string& lightVariableName);
	std::string GetSpotLightColorFunctionText();
	std::string GetStaticSpotLightText(const SpotLight* directionalLight) const;
	std::string GetSpotLightColorSummationText(const std::string& lightVariableName);

	// Shadows
	std::string GetLightShadowViewMatrixUniforms();
	std::string GetLightShadowTextureUniforms();

	std::string PointLight_GetShadowCheck(const std::string& lightName);
	std::string DirectionalLight_GetShadowCheck(const std::string& lightName);
	std::string SpotLight_GetShadowCheck(const std::string& lightName);

	// Deferred Rendering
	std::string GetFragmentShaderUniforms_DeferredRendering();

	// Fragment shader variables
	std::string uniforms_;
	std::string lightUniforms_;
	std::string fragmentShaderOutsideMain_;
	std::string fragmentShaderInsideMainBegin_;
	std::string fragmentShaderInsideMainEnd_;
	std::string fragmentShaderLightAdditionsInsideMain_;
	std::string fragmentShaderVertexNormalText_;

	std::vector<std::string> pointLightNamesForShaderSampler_;
	std::vector<std::string> directionalAndSpotLightNamesForShadowCalculation_;

	std::string vertexShaderModelMatrixVariable_;

	std::string sceneFragmentShader_;

	bool isInstantiated_;
};

#endif