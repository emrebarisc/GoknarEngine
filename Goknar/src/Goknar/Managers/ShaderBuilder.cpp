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

	sceneShaderInsideMain_ += "\tvec3 lightColor = vec3(0.f, 0.f, 0.f);\n";

	const std::vector<const PointLight*>& pointLights = scene->GetPointLights();
	if (pointLights.size() > 0)
	{
		sceneShaderOutsideMain_ += GetPointLightStructText() + "\n";
		sceneShaderOutsideMain_ += GetPointLightColorFunctionText() + "\n";

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

	const std::vector<const DirectionalLight*>& directionalLights = scene->GetDirectionalLights();
	if (directionalLights.size() > 0)
	{
		sceneShaderOutsideMain_ += GetDirectionalLightStructText() + "\n";
		sceneShaderOutsideMain_ += GetDirectionalLightColorFunctionText() + "\n";

		int lightId = 1;
		for (const DirectionalLight* directionalLight : directionalLights)
		{
			if (directionalLight->GetLightMobility() == LightMobility::Static)
			{
				std::string lightVariableName = "directionalLight" + std::to_string(lightId++);

				sceneShaderOutsideMain_ += GetStaticDirectionalLightText(directionalLight, lightVariableName);
				sceneShaderInsideMain_ += "\tlightColor += CalculateDirectionalLightColor(" + lightVariableName + ");\n";
			}
		}
	}

	const std::vector<const SpotLight*>& spotLights = scene->GetSpotLights();
	if (spotLights.size() > 0)
	{
		sceneShaderOutsideMain_ += GetSpotLightStructText() + "\n";
		sceneShaderOutsideMain_ += GetSpotLightColorFunctionText() + "\n";

		int lightId = 1;
		for (const SpotLight* spotLight : spotLights)
		{
			if (spotLight->GetLightMobility() == LightMobility::Static)
			{
				std::string lightVariableName = "spotLight" + std::to_string(lightId++);

				sceneShaderOutsideMain_ += GetStaticSpotLightText(spotLight, lightVariableName);
				sceneShaderInsideMain_ += "\tlightColor += CalculateSpotLightColor(" + lightVariableName + ");\n";
			}
		}
	}

	sceneShaderInsideMain_ += "\tcolor = lightColor;\n";

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

std::string ShaderBuilder::GetPointLightStructText()
{
	return 
R"(struct PointLight
{
    vec3 position;
    vec3 intensity;
};
)";
}

std::string ShaderBuilder::GetPointLightColorFunctionText()
{
	return R"(
vec3 CalculatePointLightColor(PointLight pointLight)
{
	// To light vector
	vec3 wi = pointLight.position - fragmentPosition;
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

	color *= pointLight.intensity * inverseDistanceSquare;

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

	return  "PointLight " + lightVariableName + " = PointLight(vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ")"
														  + ", vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + "));\n";
}

std::string ShaderBuilder::GetDirectionalLightStructText()
{
	return
		R"(struct DirectionalLight
{
    vec3 direction;
    vec3 intensity;
};
)";
}

std::string ShaderBuilder::GetDirectionalLightColorFunctionText()
{
	return R"(
vec3 CalculateDirectionalLightColor(DirectionalLight directionalLight)
{
	vec3 wi = -directionalLight.direction;
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

	color *= directionalLight.intensity * inverseDistanceSquare;

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

	return  "DirectionalLight " + lightVariableName + " = DirectionalLight(vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ")"
																	  + ", vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + "));\n";

}

std::string ShaderBuilder::GetSpotLightStructText()
{
	return
		R"(struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 intensity;
	float coverageAngle;
	float falloffAngle;
};
)";
}

std::string ShaderBuilder::GetSpotLightColorFunctionText()
{
	return R"(
vec3 CalculateSpotLightColor(SpotLight spotLight)
{
	vec3 ambient = sceneAmbient * ambientReflectance;
	vec3 diffuse = vec3(0.f, 0.f, 0.f);
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = fragmentPosition - spotLight.position;
	float wiLength = length(wi);
	wi /= wiLength;

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1.f / (wiLength * wiLength);

	float cosCoverage = cos(spotLight.coverageAngle);
	float cosFalloff = cos(spotLight.falloffAngle);

	float cosTheta = dot(wi, spotLight.direction);

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

	vec3 color = ambient + (diffuse + specular) * spotLight.intensity * inverseDistanceSquare;
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

	return  "SpotLight " + lightVariableName + " = SpotLight(vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ")"
		+ ", vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ")"
		+ ", vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ")"
		+ ", " + std::to_string(coverageAngle) + ", " + std::to_string(falloffAngle) + ");\n";
}

void ShaderBuilder::CombineShader()
{
	sceneFragmentShader_ = sceneShaderOutsideMain_;
	sceneFragmentShader_ += R"(
void main()
{
)";
	sceneFragmentShader_ += sceneShaderInsideMain_ + "\n";
	sceneFragmentShader_ += "}";
}
