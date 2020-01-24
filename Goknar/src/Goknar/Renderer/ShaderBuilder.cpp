#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Material.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Renderer/Texture.h"

ShaderBuilder* ShaderBuilder::instance_ = nullptr;

void ShaderBuilder::GetShaderForMaterial(const Material* material, std::string& vertexShader, std::string& fragmentShader)
{
	const std::vector<const Texture*>* textures = material->GetShader()->GetTextures();
	size_t textureSize = textures->size();

	// Vertex Shader
	vertexShader = "";

	vertexShader += GetShaderVersionText();
	vertexShader += VS_GetVariableTexts();

	vertexShader += R"(
void main()
{)";
	vertexShader += VS_GetMain();
	vertexShader += R"(
})";
	
	// Fragment Shader
	fragmentShader = "";
	fragmentShader += FS_GetVariableTexts();
	for (size_t textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		const Texture* texture = textures->at(textureIndex);
		fragmentShader += "uniform sampler2D" + texture->GetName() + ";\n";
	}

	fragmentShader += R"(
void main()
{)";
	fragmentShader += fragmentShaderInsideMain_;
	fragmentShader += R"(
})";
}

ShaderBuilder::ShaderBuilder()
{
	shaderVersion_ = DEFAULT_SHADER_VERSION;
}

ShaderBuilder::~ShaderBuilder()
{
}

void ShaderBuilder::Init()
{
	VS_BuildScene();
	FS_BuildScene();
}

void ShaderBuilder::FS_BuildScene()
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	uniforms_ += FS_GetVariableTexts();

	fragmentShaderOutsideMain_ += GetMaterialVariables();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	fragmentShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";

	fragmentShaderInsideMain_ += "\tvec3 lightColor = sceneAmbient * ambientReflectance;\n";

	// Lights
	{
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
	}
	fragmentShaderInsideMain_ += "\t" + std::string(SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR) + " = lightColor;";

	CombineFragmentShader();
	
	IOManager::WriteFile("./DefaultSceneVertexShader.glsl", sceneVertexShader_.c_str());
	IOManager::WriteFile("./DefaultSceneFragmentShader.glsl", sceneFragmentShader_.c_str());
}

std::string ShaderBuilder::VS_GetVertexNormalText()
{
	std::string vertexNormalText = "\n";
	vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexNormalText += " = vec3(vec4(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::NORMAL) + ", 0.f) * transpose(inverse(" + SHADER_VARIABLE_NAMES::POSITIONING::TRANSFORMATION_MATRIX + ")));\n";

	return vertexNormalText;
}

std::string ShaderBuilder::FS_GetVariableTexts()
{
	std::string variableTexts = "\n";
	variableTexts += "out vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	variableTexts += ";\n";

	variableTexts += "in vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION;
	variableTexts += ";\n";

	variableTexts += "in vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	variableTexts += ";\n";

	variableTexts += "uniform vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION;
	variableTexts += ";\n";
	
	return variableTexts;
}

void ShaderBuilder::VS_BuildScene()
{
	sceneVertexShader_ = GetShaderVersionText();
	sceneVertexShader_ += VS_GetVariableTexts();
	sceneVertexShader_ += R"(
void main()
{)";
	sceneVertexShader_ += VS_GetMain();
	sceneVertexShader_ += VS_GetVertexNormalText();
	sceneVertexShader_ += R"(
}
)";
}

std::string ShaderBuilder::VS_GetMain()
{
	return R"(
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = vec3(fragmentPosition4Channel);
)";
}

std::string ShaderBuilder::VS_GetVariableTexts()
{
	std::string variableTexts = "";

	variableTexts += "\n\n\nlayout(location = 0) in vec4 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX::COLOR;
	variableTexts += ";\n";

	variableTexts += "layout(location = 1) in vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX::POSITION;
	variableTexts += ";\n";

	variableTexts += "layout(location = 2) in vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX::NORMAL;
	variableTexts += ";\n";

	variableTexts += "layout(location = 3) in vec2 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX::UV;
	variableTexts += ";\n";

	//variableTexts += "// Transformation matrix is calculated by multiplying \n// world and relative transformation matrices\n";
	variableTexts += "uniform mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::TRANSFORMATION_MATRIX;
	variableTexts += ";\n";

	variableTexts += "uniform mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_MATRIX;
	variableTexts += ";\n";

	variableTexts += "uniform mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::PROJECTION_MATRIX;
	variableTexts += ";\n";

	variableTexts += "uniform float ";
	variableTexts += SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME;
	variableTexts += ";\n";

	variableTexts += "uniform float ";
	variableTexts += SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME;
	variableTexts += ";\n";

	variableTexts += R"(
out vec3 fragmentPosition;
out vec3 vertexNormal;
)";

	return variableTexts;
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

void ShaderBuilder::CombineFragmentShader()
{
	sceneFragmentShader_ = GetShaderVersionText() + "\n\n";
	sceneFragmentShader_ += uniforms_;

	sceneFragmentShader_ += fragmentShaderOutsideMain_;
	sceneFragmentShader_ += R"(
void main()
{
)";
	sceneFragmentShader_ += fragmentShaderInsideMain_ + "\n";
	sceneFragmentShader_ += "}";
}
