#ifndef __SHADERBUILDER_H__
#define __SHADERBUILDER_H__

#include "Goknar/Core.h"

class DirectionalLight;
class Material;
class PointLight;
class SpotLight;

class GOKNAR_API ShaderBuilder
{
public:
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

	void GetShaderForMaterial(const Material* material, std::string& vertexShader, std::string& fragmentShader);

protected:

private:
	static ShaderBuilder* instance_;

	ShaderBuilder();
	~ShaderBuilder();
	
	std::string shaderVersion_;

	// Vertex shader
	void VS_BuildScene();
	std::string VS_GetVariableTexts();
	std::string VS_GetMain();
	std::string VS_GetVertexNormalText();

	std::string vertexShaderOutsideMain_;
	std::string vertexShaderInsideMain_;
	std::string sceneVertexShader_;

	// Fragment Shader
	void FS_BuildScene();
	std::string FS_GetVariableTexts();

	// Fragment shader builder getter functions
	std::string GetShaderVersionText();
	std::string GetMaterialVariables();

	void CombineFragmentShader();

	// Point Light
	std::string GetPointLightUniformTexts(const std::string& lightVariableName);
	std::string GetPointLightColorFunctionText();
	std::string GetStaticPointLightText(const PointLight* pointLight, const std::string& lightVariableName) const;
	std::string GetPointLightColorSummationText(const std::string& lightVariableName);

	// Directional Light
	std::string GetDirectionalLightUniformTexts(const std::string& lightVariableName);
	std::string GetDirectionalLightColorFunctionText();
	std::string GetStaticDirectionalLightText(const DirectionalLight* directionalLight, const std::string& lightVariableName) const;
	std::string GetDirectionalLightColorSummationText(const std::string& lightVariableName);

	// Spot Light
	std::string GetSpotLightUniformTexts(const std::string& lightVariableName);
	std::string GetSpotLightColorFunctionText();
	std::string GetStaticSpotLightText(const SpotLight* directionalLight, const std::string& lightVariableName) const;
	std::string GetSpotLightColorSummationText(const std::string& lightVariableName);

	// Fragment shader variables
	std::string uniforms_;
	std::string fragmentShaderOutsideMain_;
	std::string fragmentShaderInsideMain_;
	std::string fragmentShaderVertexNormalText_;

	std::string sceneFragmentShader_;
};

#endif