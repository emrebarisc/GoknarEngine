#ifndef __SHADERBUILDER_H__
#define __SHADERBUILDER_H__

#include "Goknar/Core.h"

class DirectionalLight;
class PointLight;
class SpotLight;

class GOKNAR_API ShaderBuilder
{
public:
	ShaderBuilder();
	~ShaderBuilder();

	void Init();

	const std::string& GetDefaultSceneVertexShader() const
	{
		return vertexShader_;
	}

	const std::string& GetDefaultSceneFragmentShader() const
	{
		return fragmentShader_;
	}

	std::string GetShaderVersion() const
	{
		return shaderVersion_;
	}

	void SetShaderVersion(const std::string& shaderVersion)
	{
		shaderVersion_ = shaderVersion;
	}



protected:

private:
	void BuildSceneVertexShader();
	void BuildSceneFragmentShader();

	void CombineShader();

	std::string shaderVersion_;

	// Vertex shader variables
	std::string vertexShaderOutsideMain_;
	std::string vertexShaderInsideMain_;
	std::string vertexShader_;

	// Fragment Shader

	// Fragment shader builder getter functions
	std::string GetShaderVersionText();
	std::string GetMaterialVariables();

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
	std::string fragmentShader_;
};

#endif