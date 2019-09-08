#ifndef __SHADERBUILDER_H__
#define __SHADERBUILDER_H__

#include "Goknar/Core.h"

class DirectionalLight;
class PointLight;
class SpotLight;

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace MATERIAL
	{
		extern const char* AMBIENT;
		extern const char* DIFFUSE;
		extern const char* SPECULAR;
		extern const char* PHONG_EXPONENT;
	}
	
	inline namespace LIGHT
	{
		extern const char* DIRECTIONAL_LIGHT;
		extern const char* POINT_LIGHT;
		extern const char* SPOT_LIGHT;
	}

	inline namespace LIGHT_KEYWORDS
	{
		extern const char* POSITION;
		extern const char* INTENSITY;
		extern const char* DIRECTION;
		extern const char* COVERAGE_ANGLE;
		extern const char* FALLOFF_ANGLE;
	}
}

extern const std::string DEFAULT_SHADER_VERSION;

class GOKNAR_API ShaderBuilder
{
public:
	ShaderBuilder();
	~ShaderBuilder();

	void Init();

	const std::string& GetSceneVertexShader() const
	{
		return sceneVertexShader_;
	}

	const std::string& GetSceneFragmentShader() const
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

protected:

private:
	void BuildSceneVertexShader();
	void BuildSceneFragmentShader();

	void CombineShader();

	std::string shaderVersion_;

	// Vertex Shader
	std::string sceneVertexShader_;

	// Fragment Shader

	// Fragment shader builder getter functions
	std::string GetShaderVersionText();
	std::string GetMaterialVariables();

	// Point Light
	std::string GetPointLightUniformTexts(const std::string& lightVariableName);
	std::string GetPointLightColorFunctionText();
	std::string GetStaticPointLightText(const PointLight* pointLight, const std::string& lightVariableName) const;

	// Directional Light
	std::string GetDirectionalLightUniformTexts(const std::string& lightVariableName);
	std::string GetDirectionalLightColorFunctionText();
	std::string GetStaticDirectionalLightText(const DirectionalLight* directionalLight, const std::string& lightVariableName) const;

	// Spot Light
	std::string GetSpotLightUniformTexts(const std::string& lightVariableName);
	std::string GetSpotLightColorFunctionText();
	std::string GetStaticSpotLightText(const SpotLight* directionalLight, const std::string& lightVariableName) const;

	// Fragment shader variables
	std::string uniforms_;
	std::string sceneFragmentShaderOutsideMain_;
	std::string sceneFragmentShaderInsideMain_;
	std::string sceneFragmentShader_;
};

#endif