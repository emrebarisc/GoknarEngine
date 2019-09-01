#include "pch.h"
#include "ShaderManager.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Goknar/Lights/PointLight.h"

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace MATERIAL
	{
		const char* AMBIENT = "ambientReflectance";
		const char* DIFFUSE = "diffuseReflectance";
		const char* SPECULAR = "specularReflectance";
		const char* PHONG_EXPONENT = "phongExponent";
	}
}

const std::string DEFAULT_SHADER_VERSION = "330 core";

ShaderManager::ShaderManager()
{
	shaderVersion_ = DEFAULT_SHADER_VERSION;
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::Init()
{
	BuildSceneVertexShader();
	BuildSceneFragmentShader();
}

void ShaderManager::BuildSceneFragmentShader()
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	sceneShaderOutsideMain_ += GetShaderVersionText() + "\n\n";
	sceneShaderOutsideMain_ += R"(
out vec3 color;
in vec3 fragmentPosition;
in vec3 vertexNormal;
uniform vec3 viewPosition;

)";

	sceneShaderOutsideMain_ += GetMaterialVariables();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	sceneShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";
	sceneShaderOutsideMain_ += GetPointLightStructText() + "\n";

	sceneShaderInsideMain_ += "\tvec3 lightColor = vec3(0.f, 0.f, 0.f);\n";

	const std::vector<const PointLight*>& pointLights = scene->GetPointLights();
	if (pointLights.size() > 0)
	{
		int lightId = 1;
		for (const PointLight* pointLight : pointLights)
		{
			if (pointLight->GetLightMobility() == LightMobility::Static)
			{
				std::string lightVariableName = "pointLight" + std::to_string(lightId++);

				sceneShaderOutsideMain_ += GetStaticPointLightText(pointLight, lightVariableName);
				sceneShaderInsideMain_ += "\tlightColor += CalculatePointLightColor(" + lightVariableName + ");\n";
			}
		}
	}

	sceneShaderOutsideMain_ += GetLightColorFunctionText() + "\n";
	sceneShaderInsideMain_ += "\tcolor = lightColor;\n";

	CombineShader();
}

void ShaderManager::BuildSceneVertexShader()
{
	sceneVertexShader_ = GetShaderVersionText();
	sceneVertexShader_ += R"(

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;

uniform mat4 modelMatrix;

out vec3 fragmentPosition;

out vec3 vertexNormal;

void main()
{
	gl_Position = MVP * vec4(position, 1.f);
	vertexNormal = mat3(transpose(inverse(modelMatrix))) * normal;
	fragmentPosition = vec3(modelMatrix * vec4(position, 1.f));
}
)";
}

std::string ShaderManager::GetShaderVersionText()
{
	return "#version " + shaderVersion_;
}

std::string ShaderManager::GetMaterialVariables()
{
	std::string materialVariableText = std::string("// Base Material Variables\n");
	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT;
	materialVariableText += ";\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE;
	materialVariableText += ";\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT;
	materialVariableText += ";\n\n";
	return materialVariableText;
}

std::string ShaderManager::GetPointLightStructText()
{
	return 
R"(struct PointLight
{
    vec3 position;
    vec3 intensity;
};
)";
}

std::string ShaderManager::GetLightColorFunctionText()
{
	return R"(
vec3 CalculatePointLightColor(PointLight light)
{
	// To light vector
	vec3 wi = light.position - fragmentPosition;
	float wiLength = length(wi);
	wi /= wiLength;

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Ambient
	vec3 color = sceneAmbient * ambientReflectance;

	// Diffuse
	float cosThetaPrime = max(0.f, dot(wi, vertexNormal));
	color += diffuseReflectance * cosThetaPrime * light.intensity * inverseDistanceSquare;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderManager::GetStaticPointLightText(const PointLight* pointLight, const std::string& lightVariableName) const
{
	const Vector3& lightPosition = pointLight->GetPosition();
	const Vector3& lightColor = pointLight->GetColor();
	float lightIntensity = pointLight->GetIntensity();

	return  "PointLight " + lightVariableName + " = PointLight(vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ")"
														  + ", vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + "));\n";
}

void ShaderManager::CombineShader()
{
	sceneFragmentShader_ = sceneShaderOutsideMain_;
	sceneFragmentShader_ += R"(
void main()
{
)";
	sceneFragmentShader_ += sceneShaderInsideMain_ + "\n";
	sceneFragmentShader_ += "}";
}
