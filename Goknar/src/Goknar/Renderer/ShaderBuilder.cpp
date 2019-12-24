#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Managers/IOManager.h"

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

const std::string DEFAULT_SHADER_VERSION = "440 core";

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

	fragmentShaderOutsideMain_ += GetMaterialVariables();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	fragmentShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";

	fragmentShaderInsideMain_ += "\tvec3 lightColor = sceneAmbient * ambientReflectance;\n";

	const std::vector<const PointLight*>& staticPointLights = scene->GetStaticPointLights();
	const std::vector<const PointLight*>& dynamicPointLights = scene->GetDynamicPointLights();
	if (staticPointLights.size() > 0 || dynamicPointLights.size() > 0)
	{
		fragmentShaderOutsideMain_ += GetPointLightColorFunctionText() + "\n";

		for (const PointLight* staticPointLight : staticPointLights)
		{
			std::string lightVariableName = staticPointLight->GetName();
			fragmentShaderOutsideMain_ += GetStaticPointLightText(staticPointLight, lightVariableName);
			fragmentShaderInsideMain_ += GetPointLightColorSummationText(lightVariableName);
		}

		for (const PointLight* dynamicPointLight : dynamicPointLights)
		{
			std::string lightVariableName = dynamicPointLight->GetName();
			uniforms_ += GetPointLightUniformTexts(lightVariableName);
			fragmentShaderInsideMain_ += GetPointLightColorSummationText(lightVariableName);
		}
	}

	const std::vector<const DirectionalLight*>& staticDirectionalLights = scene->GetStaticDirectionalLights();
	const std::vector<const DirectionalLight*>& dynamicDirectionalLights = scene->GetDynamicDirectionalLights();
	if (staticDirectionalLights.size() > 0 || dynamicDirectionalLights.size() > 0)
	{
		fragmentShaderOutsideMain_ += GetDirectionalLightColorFunctionText() + "\n";
		
		for (const DirectionalLight* staticDirectionalLight : staticDirectionalLights)
		{
			std::string lightVariableName = staticDirectionalLight->GetName();
			fragmentShaderOutsideMain_ += GetStaticDirectionalLightText(staticDirectionalLight, lightVariableName);
			fragmentShaderInsideMain_ += GetDirectionalLightColorSummationText(lightVariableName);
		}

		for (const DirectionalLight* dynamicDirectionalLight : dynamicDirectionalLights)
		{
			std::string lightVariableName = dynamicDirectionalLight->GetName();
			uniforms_ += GetDirectionalLightUniformTexts(lightVariableName);
			fragmentShaderInsideMain_ += GetDirectionalLightColorSummationText(lightVariableName);
				
		}
	}

	const std::vector<const SpotLight*>& staticSpotLights = scene->GetStaticSpotLights();
	const std::vector<const SpotLight*>& dynamicSpotLights = scene->GetDynamicSpotLights();
	if (staticSpotLights.size() > 0 || dynamicSpotLights.size() > 0)
	{
		fragmentShaderOutsideMain_ += GetSpotLightColorFunctionText() + "\n";

		for (const SpotLight* staticpotLight : staticSpotLights)
		{
			std::string lightVariableName = staticpotLight->GetName();
			fragmentShaderOutsideMain_ += GetStaticSpotLightText(staticpotLight, lightVariableName);
			fragmentShaderInsideMain_ += GetSpotLightColorSummationText(lightVariableName);
		}

		for (const SpotLight* dynamicSpotLight : dynamicSpotLights)
		{
			std::string lightVariableName = dynamicSpotLight->GetName();
			uniforms_ += GetSpotLightUniformTexts(lightVariableName);
			fragmentShaderInsideMain_ += GetSpotLightColorSummationText(lightVariableName);
		}
	}

	fragmentShaderInsideMain_ += "\tcolor = lightColor;\n";

	CombineShader();

	//std::cout << vertexShader_ << std::endl;
	//std::cout << fragmentShader_ << std::endl;

	//IOManager::WriteFile("./DefaultSceneVertexShader.glsl", vertexShader_.c_str());
	//IOManager::WriteFile("./DefaultSceneFragmentShader.glsl", fragmentShader_.c_str());
}

void ShaderBuilder::BuildSceneVertexShader()
{
	vertexShader_ = GetShaderVersionText();
	vertexShader_ += R"(

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// Transformation matrix is calculated by multiplying  
// world and relative transformation matrices
uniform mat4 transformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragmentPosition;
out vec3 vertexNormal;

void main()
{
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	fragmentPosition = vec3(fragmentPosition4Channel);
}
)";
}

std::string ShaderBuilder::GetShaderVersionText()
{
	return "#version " + shaderVersion_;
}

std::string ShaderBuilder::GetMaterialVariables()
{
	std::string materialVariableText = "// Base Material Variables\n";
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

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

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

std::string ShaderBuilder::GetPointLightColorSummationText(const std::string& lightVariableName)
{
	return "\tlightColor += CalculatePointLightColor(" +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ");\n";
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

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

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

std::string ShaderBuilder::GetDirectionalLightColorSummationText(const std::string& lightVariableName)
{
	return "\tlightColor += CalculateDirectionalLightColor(" +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ");\n";
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
	vec3 diffuse = vec3(0.f, 0.f, 0.f);
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = fragmentPosition - position;
	float wiLength = length(wi);
	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

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

	vec3 color = (diffuse + specular) * intensity * inverseDistanceSquare;
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

std::string ShaderBuilder::GetSpotLightColorSummationText(const std::string& lightVariableName)
{
	return "\tlightColor += CalculateSpotLightColor(" +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + ");\n";
}

void ShaderBuilder::CombineShader()
{
	fragmentShader_ = GetShaderVersionText() + "\n\n";
	fragmentShader_ += uniforms_;

	fragmentShader_ += fragmentShaderOutsideMain_;
	fragmentShader_ += R"(
void main()
{
)";
	fragmentShader_ += fragmentShaderInsideMain_ + "\n";
	fragmentShader_ += "}";
}
