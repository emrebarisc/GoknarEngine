#include "pch.h"
#include "ShaderBuilderNew.h"

#include "Goknar/Application.h"
#include "Goknar/Core.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"
#include "Goknar/Lights/ShadowManager/ShadowManager.h"

ShaderBuilderNew* ShaderBuilderNew::instance_ = nullptr;

ShaderBuilderNew::~ShaderBuilderNew()
{
}

std::string ShaderBuilderNew::ForwardRender_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{	
	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetMainLayouts();

	const bool isSkeletalMesh = 0 < initializationData->boneCount;

	std::string vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX);

	if (isSkeletalMesh)
	{
		vertexShader += VS_GetSkeletalMeshLayouts();
		vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable;
		vertexShader += VS_GetSkeletalMeshVariables();
		vertexShader += VS_GetSkeletalMeshUniforms(initializationData->boneCount);
	}

	vertexShader += VS_GetUniforms();

	vertexShader += FS_GetDirectionalLightStruct();
	vertexShader += FS_GetPointLightStruct();
	vertexShader += FS_GetSpotLightStruct();
	vertexShader += FS_GetLightArrayUniforms();

	vertexShader += VS_GetLightShadowViewMatrixUniforms();
	vertexShader += VS_GetLightOutputs();
	vertexShader += R"(
void main()
{
)";
	if (isSkeletalMesh)
	{
		vertexShader += VS_GetSkeletalMeshWeightCalculation();
	}
	vertexShader += VS_GetMain(vertexShaderModelMatrixVariable);
	vertexShader += VS_GetVertexNormalText();
	vertexShader += R"(
}
)";
	return vertexShader;
}

std::string ShaderBuilderNew::ForwardRender_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string fragmentShader = "#version " + shaderVersion_ + "\n\n";
	fragmentShader += FS_GetMaterialVariables();
	fragmentShader += FS_GetDirectionalLightStruct();
	fragmentShader += FS_GetPointLightStruct();
	fragmentShader += FS_GetSpotLightStruct();
	fragmentShader += FS_GetLightArrayUniforms();
	fragmentShader += FS_GetShadowMapUniforms();
	fragmentShader += FS_GetShaderTextureUniforms(initializationData, shader);
	fragmentShader += FS_GetLightSpaceFragmentPositions();

	fragmentShader += FS_GetDirectionalLightColorFunction();
	fragmentShader += FS_GetPointLightColorFunction();
	fragmentShader += FS_GetSpotLightColorFunction();

	fragmentShader += R"(
void main()
{
)";
	fragmentShader += FS_InitializeBaseColor(initializationData);
	fragmentShader += "\t vec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY) + " = vec3(0.f);\n";;
	fragmentShader += FS_GetLightCalculationIterators();
	fragmentShader += FS_GetFinalColorCalculation();
	fragmentShader += R"(
})";

	return fragmentShader;
}

std::string ShaderBuilderNew::FS_GetMaterialVariables() const
{

	std::string materialVariableText = "\n// Base Material Variables\n";

	materialVariableText += "out vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	materialVariableText += ";\n\n\n";

	materialVariableText += "//---------------------------------PBR------------------------------------\n";
	materialVariableText += "uniform vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR;
	materialVariableText += ";\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::EMMISIVE_COLOR;
	materialVariableText += ";\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::METALLIC;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION;
	materialVariableText += ";\n";
	materialVariableText += "//------------------------------------------------------------------------";
	materialVariableText += "\n\n\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT;
	materialVariableText += ";\n\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY;
	materialVariableText += ";\n";
	materialVariableText += "//------------------------------------------------------------------------";
	materialVariableText += "\n\n\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION;
	materialVariableText += ";\n";

	materialVariableText += "in mat4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX;
	materialVariableText += ";\n";

	materialVariableText += "in vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
	materialVariableText += ";\n";

	materialVariableText += "in vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE;
	materialVariableText += ";\n";

	materialVariableText += "in vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	materialVariableText += ";\n";

	materialVariableText += "in vec2 ";
	materialVariableText += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	materialVariableText += ";\n";

	return materialVariableText;
}

std::string ShaderBuilderNew::FS_GetLightSpaceFragmentPositions() const
{
	std::string lightSpaceFragmentPositions = "";

	lightSpaceFragmentPositions += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "];\n";
	lightSpaceFragmentPositions += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::POINT_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "];\n";
	lightSpaceFragmentPositions += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "];\n";

	return lightSpaceFragmentPositions;
}

std::string ShaderBuilderNew::FS_GetDirectionalLightColorFunction() const
{
	return R"(
vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = normalize(-direction);

	float normalDotLightDirection = dot(wi, )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"();

	bool isTranslucent = 0.f < )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	bool isBackface = normalDotLightDirection < 0.f;

	if(isBackface)
	{
		if(!isTranslucent)
		{
			return vec3(0.f);
		}
		else
		{
			normalDotLightDirection = -normalDotLightDirection;
		}
	}
	
	if(isTranslucent)
	{
		normalDotLightDirection = clamp(normalDotLightDirection * (1.f +)" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(), 0.f, 1.f);
	}

	vec3 diffuseColor = intensity * normalDotLightDirection;

	// To viewpoint vector
	vec3 wo = normalize()" + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION + R"( - vec3()" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"());

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), )" + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"();
	vec3 specularColor = )" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"( * cosAlphaPrimeToThePowerOfPhongExponent;
	specularColor *= max(0, normalDotLightDirection) * intensity;
	
	vec3 finalIntensity = specularColor + diffuseColor;

	if(isBackface && isTranslucent)
	{
		finalIntensity *= )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	}

	return finalIntensity;
}
)";
}

std::string ShaderBuilderNew::FS_GetPointLightColorFunction() const
{
	return R"(
vec3 CalculatePointLightColor(vec3 position, vec3 intensity, float radius)
{
	// To light vector
	vec3 wi = position - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"();
	float wiLength = length(wi);

	if(radius < wiLength)
	{
		return vec3(0.f);
	}

	wi /= wiLength;

	float normalDotLightDirection = dot()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"(, wi);
	
	bool isTranslucent = 0.f < )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	bool isBackface = normalDotLightDirection < 0.f;

	if(isBackface)
	{
		if(!isTranslucent)
		{
			return vec3(0.f);
		}
		else
		{
			normalDotLightDirection = -normalDotLightDirection;
			normalDotLightDirection = clamp(normalDotLightDirection + )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(, 0.f, 1.f);
		}
	}

	// To viewpoint vector
	vec3 wo = normalize()" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION) + R"( - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"());

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	vec3 intensityOverDistanceSquare = intensity / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, normalDotLightDirection);
	vec3 diffuseColor = cosThetaPrime * intensityOverDistanceSquare;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), )" + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"();
	vec3 specularColor = )" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"( * cosAlphaPrimeToThePowerOfPhongExponent;

	specularColor *= cosThetaPrime * intensityOverDistanceSquare;

	vec3 finalIntensity = specularColor + diffuseColor;

	if(isBackface && isTranslucent)
	{
		finalIntensity *= )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	}

	return finalIntensity;
}
)";
}

std::string ShaderBuilderNew::FS_GetSpotLightColorFunction() const
{
	return R"(
vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	float lightMultiplier = 0.f;

	// To light vector
	vec3 wi = vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"() - position;
	float wiLength = length(wi);
	wi /= wiLength;

	float normalDotLightDirection = dot(wi, )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"();
	
	bool isTranslucent = 0.f < )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	
	bool isBackface = 0.f < normalDotLightDirection;

	if(isBackface)
	{
		if(!isTranslucent)
		{
			return vec3(0.f);
		}
		else
		{
			normalDotLightDirection = -normalDotLightDirection;
			normalDotLightDirection = -clamp(-normalDotLightDirection + )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(, 0.f, 1.f);
		}
	}
	
	vec3 intensityOverDistanceSquare = intensity / (wiLength * wiLength);

	vec3 diffuseColor = -normalDotLightDirection * intensityOverDistanceSquare;

	float cosCoverage = cos(coverageAngle);
	float cosFalloff = cos(falloffAngle);
	float cosTheta = abs(dot(wi, direction));
	
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
		lightMultiplier = pow((cosTheta - cosCoverage) / (cosFalloff - cosCoverage), 4);
	}

	// To viewpoint vector
	vec3 wo = normalize()" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION) + R"( - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"());

	// Half vector
	vec3 halfVector = (-wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"(, halfVector)), )" + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"();
	vec3 specularColor = )" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"( * cosAlphaPrimeToThePowerOfPhongExponent;

	specularColor *= intensityOverDistanceSquare;
	
	vec3 finalIntensity = (specularColor + diffuseColor) * lightMultiplier;

	if(isBackface && isTranslucent)
	{
		finalIntensity *= )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
	}

	return finalIntensity;
}
)";
}

std::string ShaderBuilderNew::FS_GetDirectionalLightStruct() const
{
	std::string directionalLightStruct =
		R"(
struct )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_STRUCT_NAME) + R"(
{
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(;
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
};
)";

	return directionalLightStruct;
}

std::string ShaderBuilderNew::FS_GetPointLightStruct() const
{
	std::string pointLightStruct =
		R"(
struct )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_STRUCT_NAME) + R"(
{
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"( ;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
};

)";
	return pointLightStruct;
}

std::string ShaderBuilderNew::FS_GetSpotLightStruct() const
{
	std::string spotLightStruct =
		R"(
struct )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_STRUCT_NAME) + R"(
{
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + R"(;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"(;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
};
)";
	return spotLightStruct;
}

std::string ShaderBuilderNew::FS_GetLightArrayUniforms() const
{
	return R"(
layout (std140, binding = )" + std::to_string(DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX) + R"() uniform )" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_UNIFORM_NAME + R"(
{
	)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
	int )" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE + R"(;
};

layout (std140, binding = )" + std::to_string(POINT_LIGHT_UNIFORM_BIND_INDEX) + R"() uniform )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_UNIFORM_NAME + R"( 
{
	)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
	int )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE + R"(;
};

layout (std140, binding = )" + std::to_string(SPOT_LIGHT_UNIFORM_BIND_INDEX) + R"() uniform )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_UNIFORM_NAME + R"(
{
	)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
	int )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE + R"(;
};
)";
}

std::string ShaderBuilderNew::FS_GetShadowMapUniforms() const
{
	return R"(
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_UNIFORM_NAME) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
uniform samplerCubeShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_UNIFORM_NAME) + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_UNIFORM_NAME) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
)";
}

std::string ShaderBuilderNew::FS_GetShaderTextureUniforms(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string uniforms = "";

	const std::vector<const Texture*>* textures = shader->GetTextures();

	std::vector<const Texture*>::const_iterator textureIterator = textures->cbegin();
	while (textureIterator != textures->cend())
	{
		const Texture* texture = *textureIterator;

		switch (texture->GetTextureUsage())
		{
		case TextureUsage::Diffuse:
			if (initializationData->baseColor.empty())
			{
				initializationData->baseColor = FS_GetDiffuseTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::Normal:
			if (initializationData->normal.empty())
			{
				initializationData->normal = FS_GetNormalTextureSampling(texture->GetName());
			}
			break;
		default:
			break;
		}

		uniforms += "uniform sampler2D " + texture->GetName() + ";\n";

		++textureIterator;
	}

	return uniforms;
}

std::string ShaderBuilderNew::FS_GetDiffuseTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + "); ");
}

std::string ShaderBuilderNew::FS_GetNormalTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ") * 0.5f + vec4(0.5f); ");
}

std::string ShaderBuilderNew::FS_GetLightCalculationIterators() const
{
	return R"(
	for(int directionalLightIndex = 0; directionalLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++directionalLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"()
		{
			vec3 lightSpaceScreenCoordinate = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS + "[directionalLightIndex].xyz / " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([directionalLightIndex].w;

			if(	0.f <= lightSpaceScreenCoordinate.x && lightSpaceScreenCoordinate.x <= 1.f &&
				0.f <= lightSpaceScreenCoordinate.y && lightSpaceScreenCoordinate.y <= 1.f)
			{
				float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = textureProj()" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_UNIFORM_NAME + "[directionalLightIndex], " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([directionalLightIndex]);

				if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
				{
					vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY +
		" = CalculateDirectionalLightColor(\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();

					)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " * " + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
				}
			}
			else
			{
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + " CalculateDirectionalLightColor(\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + " CalculateDirectionalLightColor(\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
		}
	}

	for(int pointLightIndex = 0; pointLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++pointLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"()
		{
			vec3 )" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + ".xyz + " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + " * 0.025f - " + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
			float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = texture()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_UNIFORM_NAME + "[pointLightIndex], vec4(" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"(, 0.15f)).x;
			)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( *= )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"(;
			if(length()" + std::string(SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR) + ") < " + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
			{
				vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + " = CalculatePointLightColor(\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", \n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", \n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " * " + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"(;
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" +
		"CalculatePointLightColor(\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", \n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", \n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
		}
	}

	for(int spotLightIndex = 0; spotLightIndex < )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE + R"(; ++spotLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"()
		{
			vec3 lightSpaceScreenCoordinate = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + "[spotLightIndex].xyz / " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([spotLightIndex].w;

			if(	0.f <= lightSpaceScreenCoordinate.x && lightSpaceScreenCoordinate.x <= 1.f &&
				0.f <= lightSpaceScreenCoordinate.y && lightSpaceScreenCoordinate.y <= 1.f)
			{
				float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = textureProj()" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_UNIFORM_NAME + "[spotLightIndex], " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([spotLightIndex]);

				if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
				{
					vec3 )" + CURRENT_LIGHT_INTENSITY + " = CalculateSpotLightColor(\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ",\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ",\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ",\n" +
		"\t\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"(); 
					)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " * " + CURRENT_LIGHT_INTENSITY + R"(;
				}
			}
			else
			{
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + " CalculateSpotLightColor(\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ",\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ",\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ",\n" +
		"\t\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + " CalculateSpotLightColor(\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ",\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ",\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ",\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ",\n" +
		"\t\t\t\t" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
		}
	}
)";
}

std::string ShaderBuilderNew::FS_InitializeBaseColor(MaterialInitializationData* initializationData) const
{
	std::string result = std::string("\tvec4 ") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + " = ";

	if (!initializationData->baseColor.empty())
	{
		return result + initializationData->baseColor;
	}

	return result + SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR + ";";
}

std::string ShaderBuilderNew::FS_GetFinalColorCalculation() const
{
	return std::string("\t") + SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR + " = vec4(" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + ", 1.f) * " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + ";";
}

std::string ShaderBuilderNew::VS_GetMainLayouts() const
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

std::string ShaderBuilderNew::VS_GetSkeletalMeshLayouts() const
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


std::string ShaderBuilderNew::VS_GetSkeletalMeshVariables() const
{
	std::string variables = "";

	variables += "#define ";
	variables += SHADER_VARIABLE_NAMES::SKELETAL_MESH::MAX_BONE_SIZE_MACRO;
	variables += " ";
	variables += std::to_string(MAX_BONE_SIZE_PER_VERTEX);
	variables += "\n";

	return variables;
}

std::string ShaderBuilderNew::VS_GetSkeletalMeshUniforms(int boneCount) const
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

std::string ShaderBuilderNew::VS_GetUniforms() const
{
	std::string uniforms = "";

	uniforms += "uniform mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX;
	uniforms += ";\n";

	uniforms += "uniform mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX;
	uniforms += ";\n";

	uniforms += "uniform float ";
	uniforms += SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME;
	uniforms += ";\n";

	uniforms += "uniform float ";
	uniforms += SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME;
	uniforms += ";\n\n";

	uniforms += "out mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX;
	uniforms += ";\n";

	uniforms += "out vec4 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
	uniforms += ";\n";

	uniforms += "out vec4 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE;
	uniforms += ";\n";

	uniforms += "out vec3 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	uniforms += ";\n";

	uniforms += "out vec2 ";
	uniforms += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	uniforms += ";\n";

	return uniforms;
}

std::string ShaderBuilderNew::VS_GetLightShadowViewMatrixUniforms() const
{
	return R"(

uniform mat4 )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_VIEW_MATRIX_ARRAY_NAME) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
uniform mat4 )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_VIEW_MATRIX_ARRAY_NAME + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
uniform mat4 )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_VIEW_MATRIX_ARRAY_NAME + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;

)";
}

std::string ShaderBuilderNew::VS_GetLightOutputs() const
{
	return R"(

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
out vec4 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::POINT_LIGHT_SPACE_FRAGMENT_POSITIONS + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
out vec4 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;

)";
}

std::string ShaderBuilderNew::VS_GetSkeletalMeshWeightCalculation() const
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
	weightCalculation += " += ";
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

std::string ShaderBuilderNew::VS_GetMain(const std::string& vertexShaderModelMatrixVariable) const
{
	std::string vsMain = R"(
	)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable + R"(;
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"( = vec4(position, 1.f) * )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";" + R"(
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + R"(;
	)";

	vsMain += VS_GetLightSpaceFragmentPositionCalculations();

	vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
)";

	vsMain += std::string("\t") + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = vec2(" + SHADER_VARIABLE_NAMES::VERTEX::UV + ".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y); \n";

	return vsMain;
}

std::string ShaderBuilderNew::VS_GetLightSpaceFragmentPositionCalculations() const
{
	return R"(
	
	for(int directionalLightIndex = 0; directionalLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++directionalLightIndex)
	{
		)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS + "[directionalLightIndex] = " + 
		SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_VIEW_MATRIX_ARRAY_NAME + R"([directionalLightIndex];
	}

	for(int spotLightIndex = 0; spotLightIndex < )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE + R"(; ++spotLightIndex)
	{
		)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + "[spotLightIndex] = " +
		SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_VIEW_MATRIX_ARRAY_NAME + R"([spotLightIndex];
	}

)";
}

std::string ShaderBuilderNew::VS_GetVertexNormalText() const
{
	std::string vertexNormalText = "\n\t";
	vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexNormalText += " = normalize(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::NORMAL) + " * transpose(inverse(mat3(" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX + "))));\n";

	return vertexNormalText;
}
