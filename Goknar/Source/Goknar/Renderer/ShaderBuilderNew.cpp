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
#include "Goknar/Lights/LightManager/LightManager.h"

ShaderBuilderNew* ShaderBuilderNew::instance_ = nullptr;

ShaderBuilderNew::~ShaderBuilderNew()
{
}

std::string ShaderBuilderNew::General_FS_GetScript(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
{
	std::string fragmentShader = "#version " + shaderVersion_ + "\n\n";
	fragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
	fragmentShader += fragmentShaderInitializationData.outputVariables;

	bool includeLightOperations =
		fragmentShaderInitializationData.renderPassType == RenderPassType::Forward ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::Deferred;

	if (fragmentShaderInitializationData.renderPassType == RenderPassType::Deferred)
	{
		fragmentShader += DeferredRenderPass_GetGBufferTextureUniforms();
		fragmentShader += DeferredRenderPass_GetGBufferVariables();
		fragmentShader += VS_GetLightShadowViewMatrixUniforms();
	}

	if (includeLightOperations)
	{
		fragmentShader += FS_GetDirectionalLightStruct();
		fragmentShader += FS_GetPointLightStruct();
		fragmentShader += FS_GetSpotLightStruct();

		fragmentShader += FS_GetLightArrayUniforms();
		fragmentShader += FS_GetShadowMapUniforms();
		fragmentShader += FS_GetLightSpaceFragmentPositions(fragmentShaderInitializationData);

		fragmentShader += FS_GetDirectionalLightColorFunction();
		fragmentShader += FS_GetPointLightColorFunction();
		fragmentShader += FS_GetSpotLightColorFunction();
	}

	fragmentShader += General_FS_GetShaderTextureUniforms(fragmentShaderInitializationData.materialInitializationData, fragmentShaderInitializationData.shader);

	if (fragmentShaderInitializationData.materialInitializationData && !fragmentShaderInitializationData.materialInitializationData->fragmentShaderFunctions.empty())
	{
		fragmentShader += fragmentShaderInitializationData.materialInitializationData->fragmentShaderFunctions;
	}

	if (fragmentShaderInitializationData.materialInitializationData && !fragmentShaderInitializationData.materialInitializationData->fragmentShaderUniforms.empty())
	{
		fragmentShader += fragmentShaderInitializationData.materialInitializationData->fragmentShaderUniforms;
	}

	fragmentShader += R"(
void main()
{
)";
	if (fragmentShaderInitializationData.renderPassType == RenderPassType::Forward ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::GeometryBuffer)
	{
		fragmentShader += FS_InitializeBaseColor(fragmentShaderInitializationData.materialInitializationData);
		fragmentShader += FS_InitializeEmmisiveColor(fragmentShaderInitializationData.materialInitializationData);
	}
	else if(fragmentShaderInitializationData.renderPassType == RenderPassType::Deferred)
	{
		fragmentShader += DeferredRenderPass_GetGBufferVariableAssignments();
		fragmentShader += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (includeLightOperations)
	{
		fragmentShader += "\t vec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY) + " = " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + "; \n";;
		fragmentShader += FS_GetLightCalculationIterators();
	}
	fragmentShader += fragmentShaderInitializationData.outputVariableAssignments;
	fragmentShader += R"(
})";

	return fragmentShader;
}

std::string ShaderBuilderNew::General_VS_GetScript(const VertexShaderInitializationData& vertexShaderInitializationData) const
{
	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetMainLayouts();

	std::string vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX);

	if (0 < vertexShaderInitializationData.materialInitializationData->boneCount)
	{
		vertexShader += VS_GetSkeletalMeshLayouts();
		vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable;
		vertexShader += VS_GetSkeletalMeshVariables();
		vertexShader += VS_GetSkeletalMeshUniforms(vertexShaderInitializationData.materialInitializationData->boneCount);
	}

	vertexShader += VS_GetUniforms();

	bool includeLightOperations = vertexShaderInitializationData.renderPassType == RenderPassType::Forward;

	if (includeLightOperations)
	{
		vertexShader += FS_GetDirectionalLightStruct();
		vertexShader += FS_GetPointLightStruct();
		vertexShader += FS_GetSpotLightStruct();
		vertexShader += FS_GetLightArrayUniforms();

		vertexShader += VS_GetLightShadowViewMatrixUniforms();
		vertexShader += VS_GetLightOutputs();
	}

	if (!vertexShaderInitializationData.materialInitializationData->vertexShaderFunctions.empty())
	{
		vertexShader += vertexShaderInitializationData.materialInitializationData->vertexShaderFunctions;
	}

	if (!vertexShaderInitializationData.materialInitializationData->vertexShaderUniforms.empty())
	{
		vertexShader += vertexShaderInitializationData.materialInitializationData->vertexShaderUniforms;
	}

	vertexShader += R"(
void main()
{
)";
	if (0 < vertexShaderInitializationData.materialInitializationData->boneCount)
	{
		vertexShader += VS_GetSkeletalMeshWeightCalculation();
	}
	vertexShader += VS_GetMain(vertexShaderInitializationData, vertexShaderModelMatrixVariable);
	vertexShader += R"(
}
)";

	return vertexShader;
}

std::string ShaderBuilderNew::ForwardRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Forward;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilderNew::ForwardRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = initializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::Forward;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilderNew::GeometryBufferPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::GeometryBuffer;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilderNew::GeometryBufferPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string outputVariables = GeometryBufferPass_GetOutputVariables();
	std::string outputVariableAssignments = GeometryBufferPass_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = initializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::GeometryBuffer;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilderNew::ShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Shadow;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilderNew::ShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	shadowPassFragmentShader += R"(
void main()
{
})";
	return shadowPassFragmentShader;
}

std::string ShaderBuilderNew::PointShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::PointLightShadow;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilderNew::PointShadowPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string pointShadowPassGeometryShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	pointShadowPassGeometryShader += R"(
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 )" + std::string(SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY) + R"([6];

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;

void main()
{
	for(int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i)
		{
			)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"( = gl_in[i].gl_Position;
			gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " * " + SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY + R"([face];
			EmitVertex();
		}    
		EndPrimitive();
	}
} 
)";
	return pointShadowPassGeometryShader;
}

std::string ShaderBuilderNew::PointShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	shadowPassFragmentShader += R"(

in vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;

uniform vec3 )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION + R"(;
uniform float )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS + R"(;

void main()
{
	float lightDistance = length()" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"(.xyz - )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION + R"();
    lightDistance /= )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS + R"(;
	gl_FragDepth = lightDistance;
})";
	return shadowPassFragmentShader;
}

std::string ShaderBuilderNew::DeferredRenderPass_GetVertexShaderScript()
{
	return 
		R"(#version )" + std::string(DEFAULT_SHADER_VERSION) + "\n" +
		VS_GetMainLayouts() + 
		R"(
out vec2 )" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + R"(;

void main()
{
	gl_Position = vec4()" + SHADER_VARIABLE_NAMES::VERTEX::POSITION + R"(, 1.f);
	)" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + R"( = 0.5f * gl_Position.xy + vec2(0.5f);
})";
}

std::string ShaderBuilderNew::DeferredRenderPass_GetFragmentShaderScript()
{
	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.renderPassType = RenderPassType::Deferred;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilderNew::FS_GetOutputVariables() const
{
	std::string output = "\n// Base Material Variables\n";

	output += "out vec4 ";
	output += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	output += ";\n\n\n";

	return output;
}

std::string ShaderBuilderNew::FS_GetOutputVariableAssignments() const
{
	return std::string("\t") + SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR + " = vec4(" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + ", 1.f) * " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + ";";;
}

std::string ShaderBuilderNew::GeometryBufferPass_GetOutputVariables() const
{
	std::string variables = R"(

layout(location = 0) out vec3 )" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION) + R"(;
layout(location = 1) out vec4 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
layout(location = 2) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
layout(location = 3) out vec4 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG + R"(;
layout(location = 4) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMMISIVE_COLOR + R"(;

)";

	return variables;
}

std::string ShaderBuilderNew::GeometryBufferPass_GetOutputVariableAssignments() const
{
	std::string assignments = 
R"(
	)" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG) + " = vec4(" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + ", " + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"();
	)" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION + " = " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"(.xyz;
	)" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + " = vec4(" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + ", " + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"();
	)" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + " = " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.xyz;
	)" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMMISIVE_COLOR + " = " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + R"(;
)";
	return assignments;
}

std::string ShaderBuilderNew::DeferredRenderPass_GetGBufferTextureUniforms() const
{
	return R"(
uniform sampler2D )" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION) + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMMISIVE_COLOR + R"(;
)";
}

std::string ShaderBuilderNew::DeferredRenderPass_GetGBufferVariables() const
{
	return R"(
vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
vec4 )" + SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR + R"(;
vec3 )" + SHADER_VARIABLE_NAMES::MATERIAL::EMMISIVE_COLOR + R"(;
vec3 )" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"(;
vec3 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + R"(;
float )" + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"(;
float )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
)";
}

std::string ShaderBuilderNew::DeferredRenderPass_GetGBufferVariableAssignments() const
{
	return R"(
	vec4 )" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR) + R"( = texture()" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(, )" + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"();

	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"( = vec4(texture()" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"().xyz, 1.f);

	vec4 fragmentNormalAndPhongExponent = texture()" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"();
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + R"(= fragmentNormalAndPhongExponent.xyz;
	)" + SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT + R"(= pow(2.f, fragmentNormalAndPhongExponent.a);

	vec4 specularAndTranslucency = texture()" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR_PHONG + R"(, )" + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"();
	)" + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + R"( = specularAndTranslucency.xyz;
	)" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"( = specularAndTranslucency.a;

	vec3 )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + R"( = texture()" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMMISIVE_COLOR + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"().xyz;
)";
}

std::string ShaderBuilderNew::General_FS_GetMaterialVariables(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
{
	std::string materialVariableText = "\n// Base Material Variables\n";

	materialVariableText += "//---------------------------------PBR------------------------------------\n";

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
	materialVariableText += "//------------------------------------------------------------------------";
	materialVariableText += "\n\n\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME;
	materialVariableText += ";\n";

	materialVariableText += "uniform float ";
	materialVariableText += SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME;
	materialVariableText += ";\n\n";

	materialVariableText += "uniform vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION;
	materialVariableText += ";\n";

	materialVariableText += "in mat4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX;
	materialVariableText += ";\n";

	materialVariableText += "in vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE;
	materialVariableText += ";\n";

	materialVariableText += "in vec2 ";
	materialVariableText += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	materialVariableText += ";\n";

	if( fragmentShaderInitializationData.renderPassType == RenderPassType::Forward ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::GeometryBuffer)
	{
		materialVariableText += "in vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
		materialVariableText += ";\n";

		materialVariableText += "uniform vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "uniform vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR;
		materialVariableText += ";\n";

		materialVariableText += "uniform vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::EMMISIVE_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "in vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
		materialVariableText += ";\n";

		materialVariableText += "in vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT;
		materialVariableText += ";\n\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY;
		materialVariableText += ";\n";
	}

	return materialVariableText;
}

std::string ShaderBuilderNew::FS_GetLightSpaceFragmentPositions(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
{
	std::string variableTypes = "in vec4 ";
	if (fragmentShaderInitializationData.renderPassType == RenderPassType::Deferred)
	{
		variableTypes = "vec4 ";
	}

	std::string lightSpaceFragmentPositions = "";

	lightSpaceFragmentPositions += variableTypes + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "];\n";
	lightSpaceFragmentPositions += variableTypes + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "];\n";

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
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
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
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
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
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
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
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
uniform samplerCubeShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
)";
}

std::string ShaderBuilderNew::General_FS_GetShaderTextureUniforms(MaterialInitializationData* initializationData, const Shader* shader) const
{
	if (!shader)
	{
		return "";
	}

	std::string uniforms = "";

	const std::vector<const Texture*>* textures = shader->GetTextures();

	std::vector<const Texture*>::const_iterator textureIterator = textures->cbegin();
	while (textureIterator != textures->cend())
	{
		const Texture* texture = *textureIterator;

		switch (texture->GetTextureUsage())
		{
		case TextureUsage::Diffuse:
			if (initializationData && initializationData->baseColor.result.empty())
			{
				initializationData->baseColor.result = General_FS_GetDiffuseTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::Normal:
			if (initializationData && initializationData->fragmentNormal.result.empty())
			{
				initializationData->fragmentNormal.result = General_FS_GetNormalTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::Emmisive:
			if (initializationData && initializationData->fragmentNormal.result.empty())
			{
				initializationData->emmisiveColor.result = General_FS_GetEmmisiveTextureSampling(texture->GetName());
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

std::string ShaderBuilderNew::General_FS_GetDiffuseTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + "); ");
}

std::string ShaderBuilderNew::General_FS_GetNormalTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ") * 0.5f + vec4(0.5f); ");
}

std::string ShaderBuilderNew::General_FS_GetEmmisiveTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").xyz; ");
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
				float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = textureProj()" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME + "[directionalLightIndex], " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([directionalLightIndex]);
				)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " += " + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
				)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " = clamp(" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"(, 0.f, 1.f);
				if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
				{
					vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + " = CalculateDirectionalLightColor(\n" + R"(
						)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
						)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();

					)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " * " + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
				}
			}
			else
			{
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateDirectionalLightColor(
					)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateDirectionalLightColor(
				)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
		}
	}

	for(int pointLightIndex = 0; pointLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++pointLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"()
		{
			vec3 )" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + ".xyz + " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + " * 0.025f - " + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
			float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = texture()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME + "[pointLightIndex], vec4(" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"(, 0.025f)).x;
			)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( *= )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"(;
			if(length()" + std::string(SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR) + ") < " + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
			{
				vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + " = CalculatePointLightColor(" + R"(
					)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
			}
			else
			{	
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;			
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculatePointLightColor()" + R"(
				)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION  + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY  + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
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
				float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"( = textureProj()" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME + "[spotLightIndex], " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + R"([spotLightIndex]);
				)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " += " + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
				)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " = clamp(" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"(, 0.f, 1.f);
				if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + R"()
				{
					vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + " = CalculateSpotLightColor(" + R"(
						)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
						)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
						)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
						)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + R"(,
						)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"(); 
					)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE + " * " + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
				}
			}
			else
			{
				)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateSpotLightColor(
					)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + R"(,
					)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
			}
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateSpotLightColor(
				)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + R"(,
				)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
		}
	}
)";
}

std::string ShaderBuilderNew::FS_InitializeBaseColor(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->baseColor.calculation.empty())
	{
		result += initializationData->baseColor.calculation + "\n";
	}
	
	result += std::string("\n\tvec4 ") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + " = ";

	if (initializationData && !initializationData->baseColor.result.empty())
	{
		result += initializationData->baseColor.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR) + ";";
	}

	if (initializationData && initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked)
	{
		result += "\tif (" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR) + ".a < 0.5f) discard;\n";
	}

	return result;
}

std::string ShaderBuilderNew::FS_InitializeEmmisiveColor(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->emmisiveColor.calculation.empty())
	{
		result += initializationData->emmisiveColor.calculation + "\n";
	}
	
	result += std::string("\n\tvec3 ") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + " = ";

	if (initializationData && !initializationData->emmisiveColor.result.empty())
	{
		result += initializationData->emmisiveColor.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::EMMISIVE_COLOR) + ";";
	}

	return result;
}

std::string ShaderBuilderNew::VS_GetMainLayouts() const
{
	std::string layouts = "\n\n";

	layouts += "layout(location = 0) in vec4 ";
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
	std::string layouts = "\n\n";

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
	std::string variables = "\n\n";

	variables += "#define ";
	variables += SHADER_VARIABLE_NAMES::SKELETAL_MESH::MAX_BONE_SIZE_MACRO;
	variables += " ";
	variables += std::to_string(MAX_BONE_SIZE_PER_VERTEX);
	variables += "\n";

	return variables;
}

std::string ShaderBuilderNew::VS_GetSkeletalMeshUniforms(int boneCount) const
{
	std::string uniforms = "\n\n";

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
	std::string uniforms = "\n\n";

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

	uniforms += "out vec4 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
	uniforms += ";\n";

	uniforms += "out vec2 ";
	uniforms += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	uniforms += ";\n";

	return uniforms;
}

std::string ShaderBuilderNew::VS_GetLightShadowViewMatrixUniforms() const
{
	return R"(
layout (std140, binding = )" + std::to_string(DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX) + R"() uniform )" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_NAME + R"( 
{
	mat4 )" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_VIEW_MATRIX_ARRAY_NAME + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
};

layout (std140, binding = )" + std::to_string(SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX) + R"() uniform )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_VIEW_MATRIX_UNIFORM_NAME + R"( 
{
	mat4 )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_VIEW_MATRIX_ARRAY_NAME + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
};
)";
}

std::string ShaderBuilderNew::VS_GetLightOutputs() const
{
	return R"(

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
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

std::string ShaderBuilderNew::VS_GetMain(const VertexShaderInitializationData& vertexShaderInitializationData, const std::string& vertexShaderModelMatrixVariable) const
{
	std::string vsMain = VS_GetPosition();
	vsMain += VS_GetPositionOffset(vertexShaderInitializationData.materialInitializationData);

	vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable + ";\n";
	vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"( = vec4()" + SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION + R"(, 1.f)* )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";\n";
	
	bool isPointLightShadowPass = vertexShaderInitializationData.renderPassType == RenderPassType::PointLightShadow;
	
	if (!isPointLightShadowPass)
	{
		vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
	}

	if (vertexShaderInitializationData.renderPassType == RenderPassType::Forward)
	{
		vsMain += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (!isPointLightShadowPass)
	{
		vsMain += VS_GetUV(vertexShaderInitializationData.materialInitializationData);
		vsMain += VS_GetVertexNormalText(vertexShaderInitializationData.materialInitializationData);
		vsMain += VS_GetVertexColorText();

		vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
	)";
	}
	else
	{
		vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
	)";	
	}

	return vsMain;
}

std::string ShaderBuilderNew::VS_GetPosition() const
{
	return "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " = " + std::string(SHADER_VARIABLE_NAMES::VERTEX::POSITION) + ";\n";
}

std::string ShaderBuilderNew::VS_GetPositionOffset(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (!initializationData->vertexPositionOffset.result.empty())
	{
		result += initializationData->vertexPositionOffset.calculation + "\n";
		result += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " += " + initializationData->vertexPositionOffset.result + ";\n";
	}

	return result;
}

std::string ShaderBuilderNew::VS_GetUV(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (!initializationData->uv.calculation.empty())
	{
		result += initializationData->uv.calculation;
	}

	result += std::string("\t") + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = ";

	if (!initializationData->uv.result.empty())
	{
		result += initializationData->uv.result;
	}
	else
	{
		result += std::string("vec2(") + SHADER_VARIABLE_NAMES::VERTEX::UV + ".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y); \n";
	}

	return result;
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

std::string ShaderBuilderNew::VS_GetVertexNormalText(MaterialInitializationData* initializationData) const
{
	std::string vertexNormalText = "";

	if (!initializationData->vertexNormal.calculation.empty())
	{
		vertexNormalText += initializationData->vertexNormal.calculation + "\n";
	}
	
	vertexNormalText += "\n\t";
	vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexNormalText += " = normalize(";
	
	if (!initializationData->vertexNormal.result.empty())
	{
		vertexNormalText += initializationData->vertexNormal.result;
	}
	else
	{
		vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX::NORMAL;
	}
	
	vertexNormalText += " * transpose(inverse(mat3(" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "))));\n";

	return vertexNormalText;
}

std::string ShaderBuilderNew::VS_GetVertexColorText() const
{
	std::string vertexColorText = "";

	vertexColorText += "\n\t";
	vertexColorText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
	vertexColorText += " = ";
	vertexColorText += SHADER_VARIABLE_NAMES::VERTEX::COLOR;
	vertexColorText += ";\n";

	return vertexColorText;
}
