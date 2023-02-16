#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Application.h"
#include "Goknar/Core.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Material.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Renderer/Texture.h"

ShaderBuilder* ShaderBuilder::instance_ = nullptr;

void ShaderBuilder::BuildShader(Mesh* mesh)
{
	GOKNAR_ASSERT(mesh);

	Material* material = mesh->GetMaterial();
	if (material)
	{
		Shader* shader = material->GetShader();
		if (shader)
		{
			std::string vertexShader;
			std::string fragmentShader;
			const std::vector<const Texture*>* textures = material->GetShader()->GetTextures();
			size_t textureSize = textures->size();

			// Vertex Shader
			vertexShader = "// DefaultVertexShader";
			vertexShader = GetShaderVersionText();
			vertexShader += VS_GetMainLayouts();

			SkeletalMesh* skeletalMesh = dynamic_cast<SkeletalMesh*>(mesh);
			if (skeletalMesh)
			{
				vertexShader += VS_GetSkeletalMeshLayouts();
				vertexShaderModelMatrixVariable_ = std::string(SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable_;
				vertexShader += VS_GetSkeletalMeshVariables();
				vertexShader += VS_GetSkeletalMeshUniforms(skeletalMesh->GetBoneSize());
			}

			vertexShader += VS_GetUniforms();
			vertexShader += R"(
void main()
{
)";
			if (skeletalMesh)
			{
				vertexShader += VS_GetSkeletalMeshWeightCalculation();
			}
			vertexShader += VS_GetMain();
			vertexShader += VS_GetVertexNormalText();
			vertexShader += R"(
}
)";

			// Fragment Shader

			std::string fragmentShaderVariables = GetShaderVersionText();
			fragmentShaderVariables += GetMaterialVariables();
			fragmentShaderVariables += FS_GetVariableTexts();
			fragmentShaderVariables += lightUniforms_;
			for (size_t textureIndex = 0; textureIndex < textureSize; textureIndex++)
			{
				const Texture* texture = textures->at(textureIndex);

				if (texture->GetTextureUsage() != TextureUsage::Normal)
				{
					fragmentShaderVariables += "uniform sampler2D " + texture->GetName() + ";\n";
				}
			}

			std::string fragmentShaderOutsideMain = fragmentShaderOutsideMain_;

			bool hasADiffuseShader = false;

			std::string fragmentShaderMain = R"(
void main()
{
)";
			for (size_t textureIndex = 0; textureIndex < textureSize; textureIndex++)
			{
				const Texture* texture = textures->at(textureIndex);
				if (texture->GetTextureUsage() == TextureUsage::Diffuse)
				{
					hasADiffuseShader = true;

					std::string textureColorVariable = texture->GetName() + "Color";

					fragmentShaderMain += std::string("\tvec4 ") + textureColorVariable + " = texture(" + texture->GetName() + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + "); \n";

					if (material->GetBlendModel() == MaterialBlendModel::Masked)
					{
						fragmentShaderMain += "\tif (" + textureColorVariable + ".a < 0.5f) discard;\n";
					}

					fragmentShaderMain += std::string("\t") + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + " = vec3(" + textureColorVariable + "); \n";
				}
			}

			fragmentShaderMain += "\n" + fragmentShaderInsideMain_;
			fragmentShaderMain += R"(
})";

			if (hasADiffuseShader)
			{
				fragmentShaderVariables += GetTextureDiffuseVariable();
			}
			else
			{
				fragmentShaderVariables += GetMaterialDiffuseVariable();
			}

			fragmentShader = fragmentShaderVariables;
			fragmentShader += fragmentShaderOutsideMain;
			fragmentShader += fragmentShaderMain;

			IOManager::WriteFile((ContentDir + mesh->GetName() + "VertexShader.glsl").c_str(), vertexShader.c_str());
			IOManager::WriteFile((ContentDir + mesh->GetName() + "FragmentShader.glsl").c_str(), fragmentShader.c_str());
			
			shader->SetVertexShaderScript(vertexShader);
			shader->SetFragmentShaderScript(fragmentShader);

			ResetVariables();
		}
	}
}

ShaderBuilder::ShaderBuilder() : 
	isInstantiated_(false)
{
	shaderVersion_ = DEFAULT_SHADER_VERSION;
}

ShaderBuilder::~ShaderBuilder()
{
}

void ShaderBuilder::ResetVariables()
{
	vertexShaderModelMatrixVariable_ = std::string(SHADER_VARIABLE_NAMES::POSITIONING::RELATIVE_TRANSFORMATION_MATRIX) + " * " + SHADER_VARIABLE_NAMES::POSITIONING::WORLD_TRANSFORMATION_MATRIX;
}

void ShaderBuilder::Init()
{
	ResetVariables();

	sceneVertexShader_ = VS_BuildScene();
	FS_BuildScene();
	isInstantiated_ = true;
}

std::string ShaderBuilder::GetShaderVersionText()
{
	return "#version " + shaderVersion_;
}

void ShaderBuilder::FS_BuildScene()
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	uniforms_ += FS_GetVariableTexts();
	uniforms_ += GetMaterialDiffuseVariable();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	fragmentShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";
	fragmentShaderInsideMain_ += "\tvec3 lightColor = sceneAmbient * ambientReflectance;\n";

	// Lights
	{
		const std::vector<PointLight*>& staticPointLights = scene->GetStaticPointLights();
		const std::vector<PointLight*>& dynamicPointLights = scene->GetDynamicPointLights();
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
				lightUniforms_ += GetPointLightUniformTexts(lightVariableName);
				fragmentShaderInsideMain_ += GetPointLightColorSummationText(lightVariableName);
			}
		}

		const std::vector<DirectionalLight*>& staticDirectionalLights = scene->GetStaticDirectionalLights();
		const std::vector<DirectionalLight*>& dynamicDirectionalLights = scene->GetDynamicDirectionalLights();
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
				lightUniforms_ += GetDirectionalLightUniformTexts(lightVariableName);
				fragmentShaderInsideMain_ += GetDirectionalLightColorSummationText(lightVariableName);

			}
		}

		const std::vector<SpotLight*>& staticSpotLights = scene->GetStaticSpotLights();
		const std::vector<SpotLight*>& dynamicSpotLights = scene->GetDynamicSpotLights();
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
				lightUniforms_ += GetSpotLightUniformTexts(lightVariableName);
				fragmentShaderInsideMain_ += GetSpotLightColorSummationText(lightVariableName);
			}
		}
	}

	fragmentShaderInsideMain_ += "\tlightColor *= " + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + ";\n";
	fragmentShaderInsideMain_ += "\t" + std::string(SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR) + " = lightColor;";

	uniforms_ = lightUniforms_;

	CombineFragmentShader();
	
	IOManager::WriteFile("./DefaultSceneVertexShader.glsl", sceneVertexShader_.c_str());
	IOManager::WriteFile("./DefaultSceneFragmentShader.glsl", sceneFragmentShader_.c_str());
}

std::string ShaderBuilder::VS_GetVertexNormalText()
{
	std::string vertexNormalText = "\n\t";
	vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexNormalText += " = normalize(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::NORMAL) + " * transpose(inverse(mat3(" + SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX + "))));\n";

	return vertexNormalText;
}

std::string ShaderBuilder::FS_GetVariableTexts()
{
	std::string variableTexts = "\n";
	variableTexts += "out vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	variableTexts += ";\n";

	variableTexts += "in vec4 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION;
	variableTexts += ";\n";

	variableTexts += "in vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	variableTexts += ";\n";

	variableTexts += "in vec2 ";
	variableTexts += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	variableTexts += ";\n";

	variableTexts += "uniform vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION;
	variableTexts += ";\n";
	
	return variableTexts;
}

std::string ShaderBuilder::VS_BuildScene()
{
	std::string vertexShader;
	vertexShader = "// DefaultVertexShader";
	vertexShader = GetShaderVersionText();
	vertexShader += VS_GetUniforms();
	vertexShader += R"(
void main()
{)";
	vertexShader += VS_GetMain();
	vertexShader += VS_GetVertexNormalText();
	vertexShader += R"(
}
)";

	return vertexShader;
}

std::string ShaderBuilder::VS_GetMain()
{
	std::string vsMain = R"(
	mat4 )" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable_ + R"(;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * )" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX) + ";" +
R"(
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = fragmentPosition4Channel;
)";

	vsMain += std::string("\t") + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = " + SHADER_VARIABLE_NAMES::VERTEX::UV + "; \n";

	return vsMain;
}

std::string ShaderBuilder::VS_GetMainLayouts()
{
	std::string layouts = "";

	layouts += "\n\n\nlayout(location = 0) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::COLOR;
	layouts += ";\n";

	layouts += "layout(location = 1) in vec3 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::POSITION;
	layouts += ";\n";

	layouts += "layout(location = 2) in vec3 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::NORMAL;
	layouts += ";\n";

	layouts += "layout(location = 3) in vec2 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::UV;
	layouts += ";\n";

	return layouts;
}

std::string ShaderBuilder::VS_GetSkeletalMeshLayouts()
{
	std::string layouts = "";

	layouts += "layout(location = 4) in ivec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::BONE_IDS;
	layouts += ";\n";

	layouts += "layout(location = 5) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::WEIGHTS;
	layouts += ";\n";

	return layouts;
}

std::string ShaderBuilder::VS_GetSkeletalMeshUniforms(int boneCount)
{
	std::string uniforms = "";

	uniforms += "uniform mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES;
	uniforms += "[";
	uniforms += std::to_string(boneCount);
	uniforms += "]";
	uniforms += ";\n";

	return uniforms;
}

std::string ShaderBuilder::VS_GetSkeletalMeshVariables()
{
	std::string variables = "";

	variables += "#define ";
	variables += SHADER_VARIABLE_NAMES::SKELETAL_MESH::MAX_BONE_SIZE_MACRO;
	variables += " ";
	variables += std::to_string(MAX_BONE_SIZE_PER_VERTEX);
	variables += "\n";

	return variables;
}

std::string ShaderBuilder::VS_GetSkeletalMeshWeightCalculation()
{
	std::string weightCalculation = "";

	weightCalculation += std::string("\t") + "mat4 ";
	weightCalculation += SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX;
	weightCalculation += " = mat4(0.f);\n";
	weightCalculation += std::string("\t") + "for(int boneIndex = 0; boneIndex < ";
	weightCalculation += SHADER_VARIABLE_NAMES::SKELETAL_MESH::MAX_BONE_SIZE_MACRO;
	weightCalculation += "; ++boneIndex)\n";
	weightCalculation += std::string("\t") + "{\n";
	weightCalculation += std::string("\t\t") + SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX;
	weightCalculation += "+= ";
	weightCalculation += SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES;
	weightCalculation += "[";
	weightCalculation += SHADER_VARIABLE_NAMES::VERTEX::BONE_IDS;
	weightCalculation += "[boneIndex]";
	weightCalculation += "]";
	weightCalculation += " * ";
	weightCalculation += SHADER_VARIABLE_NAMES::VERTEX::WEIGHTS;
	weightCalculation += "[boneIndex];\n";
	weightCalculation += std::string("\t") + "}\n";

	return weightCalculation;
}

std::string ShaderBuilder::VS_GetUniforms()
{
	std::string variableTexts = "";

	variableTexts += "uniform mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::WORLD_TRANSFORMATION_MATRIX;
	variableTexts += ";\n";

	variableTexts += "uniform mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::POSITIONING::RELATIVE_TRANSFORMATION_MATRIX;
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
out vec4 fragmentPosition;
out vec3 vertexNormal;
)";

	variableTexts += "out vec2 ";
	variableTexts += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	variableTexts += ";\n";

	return variableTexts;
}

std::string ShaderBuilder::GetMaterialVariables()
{
	std::string materialVariableText = "// Base Material Variables\n";
	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT;
	materialVariableText += ";\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT;
	materialVariableText += ";\n\n";
	return materialVariableText;
}

std::string ShaderBuilder::GetMaterialDiffuseVariable()
{
	return "uniform vec3 " + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + ";\n";
}

std::string ShaderBuilder::GetTextureDiffuseVariable()
{
	return "vec3 " + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + ";\n";
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
	vec3 wi = position - vec3(fragmentPosition);
	float wiLength = length(wi);
	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3(fragmentPosition);
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, dot(wi, vertexNormal));
	vec3 color = vec3(0.f);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= )" + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + R"( * cosThetaPrime * intensity * inverseDistanceSquare;

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

	vec3 color = vec3(0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3(fragmentPosition);
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= )" + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + R"(  * max(0, normalDotLightDirection) * intensity * inverseDistanceSquare;

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
	float lightMultiplier = 0.f;
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = vec3(fragmentPosition) - position;
	float wiLength = length(wi);
	wi /= wiLength;

	if(0.f < dot(wi, )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"()) return vec3(0.f);

	float inverseDistanceSquare = 1.f / (wiLength * wiLength);

	float cosCoverage = cos(coverageAngle);
	float cosFalloff = cos(falloffAngle);
	float cosTheta = dot(wi, direction);
	
	if(cosTheta < cosCoverage)
	{
		return vec3(0.f);
	}

	if(cosFalloff < cosTheta)
	{
		lightMultiplier = 1.f;
	}
	else
	{
		lightMultiplier = pow((cosTheta - cosCoverage) / (cosFalloff - cosCoverage), 8);
	}

	// To viewpoint vector
	vec3 wo = normalize(viewPosition - vec3(fragmentPosition));

	// Half vector
	vec3 halfVector = (-wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"(, halfVector)), phongExponent);
	specular = )" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"( * cosAlphaPrimeToThePowerOfPhongExponent;

	vec3 color = (1.f + specular) * lightMultiplier * intensity * inverseDistanceSquare;
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
	sceneFragmentShader_ = "// DefaultFragmentShader\n\n";
	sceneFragmentShader_ += GetShaderVersionText() + "\n\n";
	sceneFragmentShader_ += uniforms_;
	sceneFragmentShader_ += GetMaterialVariables();

	sceneFragmentShader_ += fragmentShaderOutsideMain_;
	sceneFragmentShader_ += R"(
void main()
{
)";
	sceneFragmentShader_ += fragmentShaderInsideMain_;
	sceneFragmentShader_ += R"(
})";
}
