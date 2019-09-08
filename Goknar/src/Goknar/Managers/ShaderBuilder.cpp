#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace MATERIAL
	{
		const char* AMBIENT = "ambientReflectance";
		const char* DIFFUSE = "diffuseReflectance";
		const char* SPECULAR = "specularReflectance";
		const char* PHONG_EXPONENT = "phongExponent";
	}

	inline namespace LIGHT
	{
		const char* DIRECTIONAL_LIGHT = "DirectionalLight";
		const char* POINT_LIGHT = "PointLight";
		const char* SPOT_LIGHT = "SpotLight";
	}

	inline namespace LIGHT_KEYWORDS
	{
		const char* POSITION = "Position";
		const char* INTENSITY = "Intensity";
		const char* DIRECTION = "Direction";
		const char* COVERAGE_ANGLE = "CoverageAngle";
		const char* FALLOFF_ANGLE = "FalloffAngle";
	}
}

const std::string DEFAULT_SHADER_VERSION = "330 core";

ShaderBuilder::ShaderBuilder()
{
	shaderVersion_ = DEFAULT_SHADER_VERSION;
}

ShaderBuilder::~ShaderBuilder()
{
}

void ShaderBuilder::Init()
{
	BuildSceneVertexShader();
	BuildSceneFragmentShader();
}

void ShaderBuilder::BuildSceneFragmentShader()
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	uniforms_ += R"(
out vec3 color;
in vec3 fragmentPosition;
in vec3 vertexNormal;
uniform vec3 viewPosition;

)";

	sceneFragmentShaderOutsideMain_ += GetMaterialVariables();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	sceneFragmentShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";

	sceneFragmentShaderInsideMain_ += "\tvec3 lightColor = vec3(0.f, 0.f, 0.f);\n";

	const std::vector<const PointLight*>& pointLights = scene->GetPointLights();
	if (pointLights.size() > 0)
	{
		sceneFragmentShaderOutsideMain_ += GetPointLightColorFunctionText() + "\n";

		int lightId = 1;
		for (const PointLight* pointLight : pointLights)
		{
			std::string lightVariableName = pointLight->GetName();

			if (pointLight->GetLightMobility() == LightMobility::Static)
			{
				sceneFragmentShaderOutsideMain_ += GetStaticPointLightText(pointLight, lightVariableName);
				sceneFragmentShaderInsideMain_ += "\tlightColor += CalculatePointLightColor(" + 
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " + 
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ");\n";
			}
			else
			{
				uniforms_ += GetPointLightUniformTexts(lightVariableName);
			}
		}
	}

	const std::vector<const DirectionalLight*>& directionalLights = scene->GetDirectionalLights();
	if (directionalLights.size() > 0)
	{
		sceneFragmentShaderOutsideMain_ += GetDirectionalLightColorFunctionText() + "\n";

		int lightId = 1;
		for (const DirectionalLight* directionalLight : directionalLights)
		{
			std::string lightVariableName = directionalLight->GetName();

			if (directionalLight->GetLightMobility() == LightMobility::Static)
			{
				sceneFragmentShaderOutsideMain_ += GetStaticDirectionalLightText(directionalLight, lightVariableName);
				sceneFragmentShaderInsideMain_ += "\tlightColor += CalculateDirectionalLightColor(" + 
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " + 
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ");\n";
			}
			else
			{
				uniforms_ += GetDirectionalLightUniformTexts(lightVariableName);
				
			}
		}
	}

	const std::vector<const SpotLight*>& spotLights = scene->GetSpotLights();
	if (spotLights.size() > 0)
	{
		sceneFragmentShaderOutsideMain_ += GetSpotLightColorFunctionText() + "\n";

		int lightId = 1;
		for (const SpotLight* spotLight : spotLights)
		{
			std::string lightVariableName = spotLight->GetName();

			if (spotLight->GetLightMobility() == LightMobility::Static)
			{
				sceneFragmentShaderOutsideMain_ += GetStaticSpotLightText(spotLight, lightVariableName);
				sceneFragmentShaderInsideMain_ += "\tlightColor += CalculateSpotLightColor(" +
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ", " +
					lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + ");\n";
			}
			else
			{
				uniforms_ += GetSpotLightUniformTexts(lightVariableName);

			}
		}
	}

	sceneFragmentShaderInsideMain_ += "\tcolor = lightColor;\n";

	CombineShader();

	//std::cout << sceneFragmentShader_ << std::endl;
}

void ShaderBuilder::BuildSceneVertexShader()
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

std::string ShaderBuilder::GetShaderVersionText()
{
	return "#version " + shaderVersion_;
}

std::string ShaderBuilder::GetMaterialVariables()
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

std::string ShaderBuilder::GetPointLightUniformTexts(const std::string& lightVariableName)
{
	return "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ";\n" + 
		   "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ";\n";
}

std::string ShaderBuilder::GetPointLightColorFunctionText()
{
	return R"(
vec3 CalculatePointLightColor(vec3 position, vec3 intensity)
{
	// To light vector
	vec3 wi = position - fragmentPosition;
	float wiLength = length(wi);
	wi /= wiLength;

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, dot(wi, vertexNormal));
	vec3 color = diffuseReflectance * cosThetaPrime;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= intensity * inverseDistanceSquare;

	// Ambient
	color += sceneAmbient * ambientReflectance;

	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderBuilder::GetStaticPointLightText(const PointLight* pointLight, const std::string& lightVariableName) const
{
	const Vector3& lightPosition = pointLight->GetPosition();
	const Vector3& lightColor = pointLight->GetColor();
	float lightIntensity = pointLight->GetIntensity();

	return "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + " = vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ");\n" +
		   "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n";
}

std::string ShaderBuilder::GetDirectionalLightUniformTexts(const std::string& lightVariableName)
{
	return "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ";\n" +
		   "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ";\n";
}

std::string ShaderBuilder::GetDirectionalLightColorFunctionText()
{
	return R"(
vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = -direction;
	float wiLength = length(wi);
	wi /= wiLength;

	float normalDotLightDirection = dot(vertexNormal, wi);

	vec3 color = diffuseReflectance * max(0, normalDotLightDirection);

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= intensity * inverseDistanceSquare;

	// Ambient
	color += sceneAmbient * ambientReflectance;

	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderBuilder::GetStaticDirectionalLightText(const DirectionalLight* directionalLight, const std::string& lightVariableName) const
{
	const Vector3& lightDirection = directionalLight->GetDirection();
	const Vector3& lightColor = directionalLight->GetColor();
	float lightIntensity = directionalLight->GetIntensity();

	return  "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + " = vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ");\n" +
			"vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n";

}

std::string ShaderBuilder::GetSpotLightUniformTexts(const std::string& lightVariableName)
{
	return  "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ";\n" + 
		  + "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ";\n" +
		  + "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ";\n" +
		  + "uniform float " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ";\n" +
		  + "uniform float " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + ";\n";
}

std::string ShaderBuilder::GetSpotLightColorFunctionText()
{
	return R"(
vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	vec3 ambient = sceneAmbient * ambientReflectance;
	vec3 diffuse = vec3(0.f, 0.f, 0.f);
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = fragmentPosition - position;
	float wiLength = length(wi);
	wi /= wiLength;

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1.f / (wiLength * wiLength);

	float cosCoverage = cos(coverageAngle);
	float cosFalloff = cos(falloffAngle);

	float cosTheta = dot(wi, direction);

	if(cosTheta > cosFalloff)
	{
		diffuse = diffuseReflectance;
	}
	else if(cosTheta > cosCoverage)
	{
		float c = pow((cosTheta - cosCoverage) / (cosFalloff - cosCoverage), 4);
		diffuse = diffuseReflectance * c;
	}

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	specular = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	vec3 color = ambient + (diffuse + specular) * intensity * inverseDistanceSquare;
	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderBuilder::GetStaticSpotLightText(const SpotLight* spotLight, const std::string& lightVariableName) const
{
	const Vector3& lightPosition = spotLight->GetPosition();
	const Vector3& lightDirection = spotLight->GetDirection();
	const Vector3& lightColor = spotLight->GetColor();
	float lightIntensity = spotLight->GetIntensity();
	float coverageAngle = spotLight->GetCoverageAngle();
	float falloffAngle = spotLight->GetFalloffAngle();

	return  "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + " = vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ");\n"
		  + "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + " = vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ");\n"
		  + "vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n"
		  + "float " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + " = " + std::to_string(coverageAngle) + ";\n"
		  + "float " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + " = " + std::to_string(falloffAngle) + ";\n";
}

void ShaderBuilder::CombineShader()
{
	sceneFragmentShader_ = GetShaderVersionText() + "\n\n";
	sceneFragmentShader_ += uniforms_;

	sceneFragmentShader_ += sceneFragmentShaderOutsideMain_;
	sceneFragmentShader_ += R"(
void main()
{
)";
	sceneFragmentShader_ += sceneFragmentShaderInsideMain_ + "\n";
	sceneFragmentShader_ += "}";
}
