#ifndef __SHADERMANAGER_H__
#define __SHADERMANAGER_H__

#include "Goknar/Core.h"

class PointLight;

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace MATERIAL
	{
		extern const char* AMBIENT;
		extern const char* DIFFUSE;
		extern const char* SPECULAR;
		extern const char* PHONG_EXPONENT;
	}
}

extern const std::string DEFAULT_SHADER_VERSION;

class GOKNAR_API ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

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
	std::string GetPointLightStructText();
	std::string GetLightColorFunctionText();
	std::string GetStaticPointLightText(const PointLight* pointLight, const std::string& lightVariableName) const;

	// Fragment shader variables
	std::string sceneShaderOutsideMain_;
	std::string sceneShaderInsideMain_;
	std::string sceneFragmentShader_;
};

#endif