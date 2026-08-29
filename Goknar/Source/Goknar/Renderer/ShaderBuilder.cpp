#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Application.h"
#include "Goknar/Core.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Model/MeshGeometry.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBindingPoints.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

ShaderBuilder* ShaderBuilder::instance_ = nullptr;

namespace
{
	constexpr const char* PARTICLE_EMISSIVE_COLOR_VARYING_NAME = "particleEmissiveColor";

	bool MaterialUsesReflectionProbe(const MaterialInitializationData* initializationData)
	{
		return initializationData && initializationData->owner && initializationData->owner->GetUsesReflectionProbe();
	}

	std::string TrimTrailingStatementTerminators(std::string expression)
	{
		while (!expression.empty() && std::isspace(static_cast<unsigned char>(expression.back())))
		{
			expression.pop_back();
		}

		while (!expression.empty() && expression.back() == ';')
		{
			expression.pop_back();
			while (!expression.empty() && std::isspace(static_cast<unsigned char>(expression.back())))
			{
				expression.pop_back();
			}
		}

		return expression;
	}

	std::string GetTextureAtlasWrappedUVComponent(const std::string& uvExpression, char componentName, TextureWrapping wrapping)
	{
		const std::string component = uvExpression + "." + componentName;

		switch (wrapping)
		{
		case TextureWrapping::REPEAT:
			return "fract(" + component + ")";
		case TextureWrapping::MIRRORED_REPEAT:
			return "(1.0 - abs(fract(" + component + " * 0.5) * 2.0 - 1.0))";
		case TextureWrapping::CLAMP_TO_EDGE:
		case TextureWrapping::CLAMP_TO_BORDER:
		default:
			return "clamp(" + component + ", 0.0, 1.0)";
		}
	}

	std::vector<const Texture*> GetDeterministicallySortedTextures(const Shader* shader)
	{
		std::vector<const Texture*> sortedTextures;
		if (!shader || !shader->GetTextures())
		{
			return sortedTextures;
		}

		for (const Texture* texture : *shader->GetTextures())
		{
			if (texture)
			{
				sortedTextures.push_back(texture);
			}
		}

		std::stable_sort(
			sortedTextures.begin(),
			sortedTextures.end(),
			[](const Texture* left, const Texture* right)
			{
				if (left->GetTextureUsage() != right->GetTextureUsage())
				{
					return static_cast<unsigned char>(left->GetTextureUsage()) < static_cast<unsigned char>(right->GetTextureUsage());
				}

				const std::string leftName = left->GetShaderUniformName();
				const std::string rightName = right->GetShaderUniformName();
				if (leftName != rightName)
				{
					return leftName < rightName;
				}

				return left->GetGUID() < right->GetGUID();
			});

		return sortedTextures;
	}

}

ShaderBuilder::~ShaderBuilder()
{
}

std::string ShaderBuilder::General_FS_GetScript(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
{
	MaterialInitializationData resolvedMaterialInitializationData(
		fragmentShaderInitializationData.materialInitializationData ? fragmentShaderInitializationData.materialInitializationData->owner : nullptr);

	FragmentShaderInitializationData resolvedFragmentShaderInitializationData(
		fragmentShaderInitializationData.outputVariables,
		fragmentShaderInitializationData.outputVariableAssignments);
	resolvedFragmentShaderInitializationData.shader = fragmentShaderInitializationData.shader;
	resolvedFragmentShaderInitializationData.renderPassType = fragmentShaderInitializationData.renderPassType;

	if (fragmentShaderInitializationData.materialInitializationData)
	{
		resolvedMaterialInitializationData = *fragmentShaderInitializationData.materialInitializationData;
		ApplyTextureBackedMaterialDefaults(&resolvedMaterialInitializationData, fragmentShaderInitializationData.shader);
		resolvedFragmentShaderInitializationData.materialInitializationData = &resolvedMaterialInitializationData;
	}

	const FragmentShaderInitializationData& shaderData = resolvedFragmentShaderInitializationData;

	std::string fragmentShader = "#version " + shaderVersion_ + "\n\n";
	fragmentShader += General_FS_GetMaterialVariables(shaderData);
	fragmentShader += shaderData.outputVariables;

	bool includeLightOperations =
		(shaderData.renderPassType == RenderPassType::Forward ||
		shaderData.renderPassType == RenderPassType::Deferred ||
		shaderData.renderPassType == RenderPassType::CubemapCapture);

	const bool includeShadowOperations =
		shaderData.renderPassType == RenderPassType::Forward ||
		shaderData.renderPassType == RenderPassType::Deferred;

	const bool includeReflectionProbeOperations =
		shaderData.renderPassType == RenderPassType::Deferred ||
		MaterialUsesReflectionProbe(shaderData.materialInitializationData);

	if (shaderData.renderPassType == RenderPassType::Deferred)
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
		if (includeReflectionProbeOperations)
		{
			fragmentShader += FS_GetReflectionProbeUniforms();
		}

		if (includeShadowOperations)
		{
			fragmentShader += FS_GetShadowMapUniforms();
			fragmentShader += FS_GetLightSpaceFragmentPositions(shaderData);
		}

		if (includeReflectionProbeOperations && shaderData.renderPassType != RenderPassType::Deferred)
		{
			fragmentShader += "float " + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::USAGE) + " = 1.f;\n";
		}

		fragmentShader += FS_GetPBRFunctions(includeReflectionProbeOperations);
		fragmentShader += FS_GetDirectionalLightColorFunction();
		fragmentShader += FS_GetPointLightColorFunction();
		fragmentShader += FS_GetSpotLightColorFunction();
	}

	fragmentShader += General_FS_GetShaderTextureUniforms(shaderData.materialInitializationData, shaderData.shader);

	if (shaderData.materialInitializationData && !shaderData.materialInitializationData->fragmentShaderFunctions.empty())
	{
		fragmentShader += shaderData.materialInitializationData->fragmentShaderFunctions;
	}

	if (shaderData.materialInitializationData && !shaderData.materialInitializationData->fragmentShaderUniforms.empty())
	{
		fragmentShader += shaderData.materialInitializationData->fragmentShaderUniforms;
	}

	fragmentShader += R"(
void main()
{
)";
	const bool isMaterialSurfacePass =
		shaderData.renderPassType == RenderPassType::Forward ||
		shaderData.renderPassType == RenderPassType::GeometryBuffer ||
		shaderData.renderPassType == RenderPassType::CubemapCapture;

	if (isMaterialSurfacePass)
	{
		fragmentShader += FS_InitializeBaseColor(shaderData.materialInitializationData);
		fragmentShader += FS_InitializeEmissiveColor(shaderData.materialInitializationData);
		fragmentShader += FS_InitializeAmbientOcclusion(shaderData.materialInitializationData);
		fragmentShader += FS_InitializeMetallic(shaderData.materialInitializationData);
		fragmentShader += FS_InitializeRoughness(shaderData.materialInitializationData);
		fragmentShader += FS_InitializeSurfaceNormal(shaderData.materialInitializationData);
	}
	else if (shaderData.renderPassType == RenderPassType::Deferred)
	{
		fragmentShader += DeferredRenderPass_GetGBufferVariableAssignments();
		fragmentShader += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (includeLightOperations)
	{
		fragmentShader += "\tInitializePBRSharedInputs();\n";
		if (shaderData.renderPassType == RenderPassType::Deferred)
		{
			fragmentShader += DeferredRenderPass_GetUnlitCheck();
		}
		else if (shaderData.renderPassType != RenderPassType::GeometryBuffer)
		{
			fragmentShader += FS_GetUnlitCheck(includeReflectionProbeOperations);
		}
		fragmentShader += "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY) + " = vec3(0.f);\n";
		fragmentShader += FS_GetLightCalculationIterators(includeShadowOperations);
	}

	fragmentShader += shaderData.outputVariableAssignments;
	fragmentShader += R"(
})";

	return fragmentShader;
}

std::string ShaderBuilder::General_VS_GetScript(const VertexShaderInitializationData& vertexShaderInitializationData) const
{
	MaterialInitializationData fallbackMaterialInitializationData(nullptr);
	VertexShaderInitializationData shaderData = vertexShaderInitializationData;
	if (!shaderData.materialInitializationData)
	{
		shaderData.materialInitializationData = &fallbackMaterialInitializationData;
	}

	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetMainLayouts();

	std::string vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX);
	const bool isInstancedStaticMesh = shaderData.meshType == MeshType::InstancedStatic;
	const bool usesGPUFoliageInstanceBuffer = shaderData.usesGPUFoliageInstanceBuffer;

	if (isInstancedStaticMesh)
	{
		vertexShader += VS_GetInstancedStaticMeshLayouts();
		vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::INSTANCE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable;
	}
	else if (usesGPUFoliageInstanceBuffer)
	{
		vertexShader += VS_GetGPUFoliageInstanceBuffer();
		vertexShaderModelMatrixVariable = "foliageInstance.transform * " + vertexShaderModelMatrixVariable;
	}

	if (0 < shaderData.materialInitializationData->boneCount)
	{
		vertexShader += VS_GetSkeletalMeshLayouts();
		vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::BONE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable;
		vertexShader += VS_GetSkeletalMeshVariables();
		vertexShader += VS_GetSkeletalMeshUniforms(shaderData.materialInitializationData->boneCount);
	}

	vertexShader += VS_GetUniforms();
	vertexShader += General_FS_GetShaderTextureUniforms(shaderData.materialInitializationData, shaderData.shader);

	bool includeLightOperations = shaderData.renderPassType == RenderPassType::Forward;

	if (includeLightOperations)
	{
		vertexShader += FS_GetDirectionalLightStruct();
		vertexShader += FS_GetPointLightStruct();
		vertexShader += FS_GetSpotLightStruct();
		vertexShader += FS_GetLightArrayUniforms();

		vertexShader += VS_GetLightShadowViewMatrixUniforms();
		vertexShader += VS_GetLightOutputs();
	}

	if (!shaderData.materialInitializationData->vertexShaderFunctions.empty())
	{
		vertexShader += shaderData.materialInitializationData->vertexShaderFunctions;
	}

	if (!shaderData.materialInitializationData->vertexShaderUniforms.empty())
	{
		vertexShader += shaderData.materialInitializationData->vertexShaderUniforms;
	}

	vertexShader += R"(
void main()
{
)";
	if (usesGPUFoliageInstanceBuffer)
	{
		vertexShader += "\tGPUFoliageInstanceData foliageInstance = foliageInstances[gl_InstanceID];\n";
	}
	if (0 < shaderData.materialInitializationData->boneCount)
	{
		vertexShader += VS_GetSkeletalMeshWeightCalculation();
	}
	if (isInstancedStaticMesh)
	{
		vertexShader += VS_GetInstancedStaticMeshTransformationMatrixCalculation();
	}
	vertexShader += VS_GetMain(shaderData, vertexShaderModelMatrixVariable);
	vertexShader += R"(
}
)";

	return vertexShader;
}

std::string ShaderBuilder::ForwardRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Forward;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::ForwardRenderPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Forward;
	vertexShaderInitializationData.meshType = MeshType::InstancedStatic;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::ForwardRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = initializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::Forward;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilder::GeometryBufferPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::GeometryBuffer;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::GeometryBufferPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::GeometryBuffer;
	vertexShaderInitializationData.meshType = MeshType::InstancedStatic;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::GeometryBufferPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string outputVariables = GeometryBufferPass_GetOutputVariables();
	std::string outputVariableAssignments = GeometryBufferPass_GetOutputVariableAssignments(initializationData);

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = initializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::GeometryBuffer;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilder::ShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Shadow;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::ShadowPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::Shadow;
	vertexShaderInitializationData.meshType = MeshType::InstancedStatic;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::ShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	MaterialInitializationData resolvedMaterialInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		resolvedMaterialInitializationData = *initializationData;
		ApplyTextureBackedMaterialDefaults(&resolvedMaterialInitializationData, shader);
		effectiveInitializationData = &resolvedMaterialInitializationData;
	}

	bool requiresAlphaTest = effectiveInitializationData &&
		effectiveInitializationData->owner &&
		(effectiveInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			effectiveInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	if (requiresAlphaTest)
	{
		FragmentShaderInitializationData fragmentShaderInitializationData("", "");
		fragmentShaderInitializationData.materialInitializationData = effectiveInitializationData;
		fragmentShaderInitializationData.shader = shader;
		fragmentShaderInitializationData.renderPassType = RenderPassType::Shadow;

		std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
		shadowPassFragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
		shadowPassFragmentShader += General_FS_GetShaderTextureUniforms(effectiveInitializationData, shader);

		if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderFunctions.empty())
		{
			shadowPassFragmentShader += effectiveInitializationData->fragmentShaderFunctions;
		}

		if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderUniforms.empty())
		{
			shadowPassFragmentShader += effectiveInitializationData->fragmentShaderUniforms;
		}

		shadowPassFragmentShader += R"(
void main()
{
)";
		shadowPassFragmentShader += FS_InitializeBaseColor(effectiveInitializationData, true);
		shadowPassFragmentShader += FS_InitializeEmissiveColor(effectiveInitializationData);
		shadowPassFragmentShader += "\n\tif (1.f < length(" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR) + ")) discard;\n";
		shadowPassFragmentShader += R"(
})";
		return shadowPassFragmentShader;
	}
	else
	{
		std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
		shadowPassFragmentShader += R"(
void main()
{
})";
		return shadowPassFragmentShader;
	}
}

std::string ShaderBuilder::PointShadowPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::PointLightShadow;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::PointShadowPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::PointLightShadow;
	vertexShaderInitializationData.meshType = MeshType::InstancedStatic;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::PointShadowPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	bool requiresAlphaTest = initializationData &&
		initializationData->owner &&
		(initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			initializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	std::string pointShadowPassGeometryShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	pointShadowPassGeometryShader += R"(
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 )" + std::string(SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY) + R"([6];

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
)";

	if (requiresAlphaTest)
	{
		pointShadowPassGeometryShader += "in mat4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "[];\n";
		pointShadowPassGeometryShader += "out mat4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "[];\n";
		pointShadowPassGeometryShader += "out vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "[];\n";
		pointShadowPassGeometryShader += "out vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "[];\n";
		pointShadowPassGeometryShader += "out vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "_GS;\n";

		pointShadowPassGeometryShader += "in float " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "[];\n";
		pointShadowPassGeometryShader += "out float " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "[];\n";
		pointShadowPassGeometryShader += "out vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "[];\n";
		pointShadowPassGeometryShader += "out vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "_GS;\n";
	}

	pointShadowPassGeometryShader += R"(
void main()
{
	for(int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i)
		{
			)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"( = gl_in[i].gl_Position;
			gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " * " + SHADER_VARIABLE_NAMES::SHADOW::POINT_LIGHT_VIEW_MATRICES_ARRAY + R"([face];
)";

	if (requiresAlphaTest)
	{
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "[i];\n";
	}

	pointShadowPassGeometryShader += R"(
			EmitVertex();
		}
		EndPrimitive();
	}
}
)";
	return pointShadowPassGeometryShader;
}

std::string ShaderBuilder::PointShadowPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	MaterialInitializationData resolvedMaterialInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		resolvedMaterialInitializationData = *initializationData;
		ApplyTextureBackedMaterialDefaults(&resolvedMaterialInitializationData, shader);
		effectiveInitializationData = &resolvedMaterialInitializationData;
	}

	bool requiresAlphaTest = effectiveInitializationData &&
		effectiveInitializationData->owner &&
		(effectiveInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			effectiveInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";

	if (requiresAlphaTest)
	{
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "_GS\n";

		FragmentShaderInitializationData fragmentShaderInitializationData("", "");
		fragmentShaderInitializationData.materialInitializationData = effectiveInitializationData;
		fragmentShaderInitializationData.shader = shader;
		fragmentShaderInitializationData.renderPassType = RenderPassType::PointLightShadow;

		shadowPassFragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
		shadowPassFragmentShader += General_FS_GetShaderTextureUniforms(effectiveInitializationData, shader);

		if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderFunctions.empty())
		{
			shadowPassFragmentShader += effectiveInitializationData->fragmentShaderFunctions;
		}

		if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderUniforms.empty())
		{
			shadowPassFragmentShader += effectiveInitializationData->fragmentShaderUniforms;
		}
	}

	shadowPassFragmentShader += "\n";
	if (!requiresAlphaTest)
	{
		shadowPassFragmentShader += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + ";\n";
	}

	shadowPassFragmentShader += R"(

uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION) + R"(;
uniform float )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS + R"(;

void main()
{
)";

	if (requiresAlphaTest)
	{
		shadowPassFragmentShader += FS_InitializeBaseColor(effectiveInitializationData, true);
		shadowPassFragmentShader += FS_InitializeEmissiveColor(effectiveInitializationData);
		shadowPassFragmentShader += "\n\tif (1.f < length(" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR) + ")) discard;\n";
	}

	shadowPassFragmentShader += R"(
	float lightDistance = length()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(.xyz - )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION + R"();
    lightDistance /= )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS + R"(;
	gl_FragDepth = lightDistance;
})";
	return shadowPassFragmentShader;
}

std::string ShaderBuilder::CubemapRenderPass_GetVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::CubemapCapture;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::CubemapRenderPass_GetInstancedStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = initializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = RenderPassType::CubemapCapture;
	vertexShaderInitializationData.meshType = MeshType::InstancedStatic;
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::CubemapRenderPass_GetGeometryShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string cubemapPassGeometryShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	cubemapPassGeometryShader += R"(
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::VIEW_MATRICES_ARRAY) + R"([6];

in mat4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + R"([];
out mat4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + R"(_GS;

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(_GS;
out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(_GS;
)";
	cubemapPassGeometryShader += "in vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "[];\n";
	cubemapPassGeometryShader += "out vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS;\n\n";
	cubemapPassGeometryShader += "in vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "[];\n";
	cubemapPassGeometryShader += "out vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS;\n\n";
	cubemapPassGeometryShader += "in vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "[];\n";
	cubemapPassGeometryShader += "out vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "_GS;\n\n";
	cubemapPassGeometryShader += "in float " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "[];\n";
	cubemapPassGeometryShader += "out float " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "_GS;\n\n";
	cubemapPassGeometryShader += "in vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "[];\n";
	cubemapPassGeometryShader += "out vec4 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS;\n";
	cubemapPassGeometryShader += R"(

void main()
{
	for(int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i)
		{
			)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + R"(_GS = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX + R"([i];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"(_GS = gl_in[i].gl_Position;
			gl_Position = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + "_GS * " + SHADER_VARIABLE_NAMES::REFLECTION_PROBE::VIEW_MATRICES_ARRAY + R"([face];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE + R"(_GS = gl_Position;
			)" + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"(_GS = )" + SHADER_VARIABLE_NAMES::TEXTURE::UV + R"([i];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + R"(_GS = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + R"([i];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT + R"(_GS = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT + R"([i];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN + R"(_GS = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN + R"([i];
			)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR + R"(_GS = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR + R"([i];
			EmitVertex();
		}
		EndPrimitive();
	}
}
)";

	return cubemapPassGeometryShader;
}

std::string ShaderBuilder::CubemapRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	MaterialInitializationData resolvedMaterialInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		resolvedMaterialInitializationData = *initializationData;
		ApplyTextureBackedMaterialDefaults(&resolvedMaterialInitializationData, shader);
		effectiveInitializationData = &resolvedMaterialInitializationData;
	}

	const bool materialUsesReflectionProbe = MaterialUsesReflectionProbe(effectiveInitializationData);

	std::string cubemapFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + "_GS\n";
	cubemapFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS\n\n";

	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = effectiveInitializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::CubemapCapture;

	cubemapFragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
	cubemapFragmentShader += outputVariables;
	cubemapFragmentShader += FS_GetDirectionalLightStruct();
	cubemapFragmentShader += FS_GetPointLightStruct();
	cubemapFragmentShader += FS_GetSpotLightStruct();
	cubemapFragmentShader += FS_GetLightArrayUniforms();
	if (materialUsesReflectionProbe)
	{
		cubemapFragmentShader += FS_GetReflectionProbeUniforms();
		cubemapFragmentShader += "float " + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::USAGE) + " = 1.f;\n";
	}
	cubemapFragmentShader += FS_GetPBRFunctions(materialUsesReflectionProbe);
	cubemapFragmentShader += FS_GetDirectionalLightColorFunction();
	cubemapFragmentShader += FS_GetPointLightColorFunction();
	cubemapFragmentShader += FS_GetSpotLightColorFunction();
	cubemapFragmentShader += General_FS_GetShaderTextureUniforms(effectiveInitializationData, shader);

	if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderFunctions.empty())
	{
		cubemapFragmentShader += effectiveInitializationData->fragmentShaderFunctions;
	}

	if (effectiveInitializationData && !effectiveInitializationData->fragmentShaderUniforms.empty())
	{
		cubemapFragmentShader += effectiveInitializationData->fragmentShaderUniforms;
	}

	cubemapFragmentShader += R"(
void main()
{
)";
	cubemapFragmentShader += FS_InitializeBaseColor(effectiveInitializationData);
	cubemapFragmentShader += FS_InitializeEmissiveColor(effectiveInitializationData);
	cubemapFragmentShader += FS_InitializeAmbientOcclusion(effectiveInitializationData);
	cubemapFragmentShader += FS_InitializeMetallic(effectiveInitializationData);
	cubemapFragmentShader += FS_InitializeRoughness(effectiveInitializationData);
	cubemapFragmentShader += FS_InitializeSurfaceNormal(effectiveInitializationData);
	cubemapFragmentShader += "\tInitializePBRSharedInputs();\n";
	cubemapFragmentShader += FS_GetUnlitCheck(materialUsesReflectionProbe);
	cubemapFragmentShader += "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY) + " = vec3(0.f);\n";
	cubemapFragmentShader += FS_GetLightCalculationIterators(false);
	cubemapFragmentShader += outputVariableAssignments;
	cubemapFragmentShader += R"(
})";

	return cubemapFragmentShader;
}

std::string ShaderBuilder::ParticleRenderPass_GetBillboardVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetUniforms();
	vertexShader += FS_GetDirectionalLightStruct();
	vertexShader += FS_GetPointLightStruct();
	vertexShader += FS_GetSpotLightStruct();
	vertexShader += FS_GetLightArrayUniforms();
	vertexShader += VS_GetLightShadowViewMatrixUniforms();
	vertexShader += VS_GetLightOutputs();

	vertexShader += R"(
layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::POSITION) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::POSITION_BUFFER_NAME) + R"(
{
	vec4 particlePositions[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::VELOCITY) + R"() readonly buffer ParticleVelocityBuffer
{
	vec4 particleVelocities[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::COLOR) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::COLOR_BUFFER_NAME) + R"(
{
	vec4 particleColors[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::LIFETIME) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::LIFETIME_BUFFER_NAME) + R"(
{
	vec2 particleLifetimes[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::ALIVE_INDEX) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::ALIVE_INDEX_BUFFER_NAME) + R"(
{
	uint aliveParticleIndices[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::END_COLOR) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::END_COLOR_BUFFER_NAME) + R"(
{
	vec4 particleEndColors[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::SIZE) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::SIZE_BUFFER_NAME) + R"(
{
	vec2 particleSizes[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::ROTATION) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::ROTATION_BUFFER_NAME) + R"(
{
	vec4 particleRotations[];
};

uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_RIGHT) + R"(;
uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_UP) + R"(;
uniform float )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::PARTICLE_SIZE) + R"(;
uniform vec2 particleSizeBySpeedRange;
uniform vec2 particleSizeBySpeedValues;
uniform vec2 particleColorBySpeedRange;
uniform vec4 particleColorBySpeedStart;
uniform vec4 particleColorBySpeedEnd;
uniform vec3 particleEmissiveColorStart;
uniform vec3 particleEmissiveColorEnd;
out vec3 )" + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + R"(;
)";

	MaterialInitializationData particleInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		particleInitializationData = *initializationData;
		if (particleInitializationData.uv.result.empty())
		{
			particleInitializationData.uv.result = "vec2(uv.x, 1.f - uv.y);";
		}

		effectiveInitializationData = &particleInitializationData;
	}
	const bool hasWorldPositionOffset = effectiveInitializationData &&
		!effectiveInitializationData->vertexPositionOffset.result.empty();

	if (effectiveInitializationData && !effectiveInitializationData->vertexShaderFunctions.empty())
	{
		vertexShader += effectiveInitializationData->vertexShaderFunctions;
	}

	if (effectiveInitializationData && !effectiveInitializationData->vertexShaderUniforms.empty())
	{
		vertexShader += effectiveInitializationData->vertexShaderUniforms;
	}

	vertexShader += R"(
void main()
{
	const vec2 particleQuadCorners[6] = vec2[](
		vec2(-1.0, -1.0),
		vec2(1.0, -1.0),
		vec2(1.0, 1.0),
		vec2(-1.0, -1.0),
		vec2(1.0, 1.0),
		vec2(-1.0, 1.0));

	uint particleIndex = aliveParticleIndices[gl_InstanceID];
	vec4 startParticleColor = particleColors[particleIndex];
	vec4 endParticleColor = particleEndColors[particleIndex];
	vec2 particleLifetime = particleLifetimes[particleIndex];
	vec2 particleSizeRange = particleSizes[particleIndex];
	vec3 particleRotation = particleRotations[particleIndex].xyz;
	float particleSpeed = length(particleVelocities[particleIndex].xyz);
	float normalizedAge = particleLifetime.y < 0.0 ? 0.0 : 1.0 - clamp(particleLifetime.x / max(particleLifetime.y, 0.0001), 0.0, 1.0);
	float normalizedSizeSpeed = clamp((particleSpeed - particleSizeBySpeedRange.x) / max(particleSizeBySpeedRange.y - particleSizeBySpeedRange.x, 0.0001), 0.0, 1.0);
	float normalizedColorSpeed = clamp((particleSpeed - particleColorBySpeedRange.x) / max(particleColorBySpeedRange.y - particleColorBySpeedRange.x, 0.0001), 0.0, 1.0);
	float currentParticleSize = mix(particleSizeRange.x, particleSizeRange.y, normalizedAge) * mix(particleSizeBySpeedValues.x, particleSizeBySpeedValues.y, normalizedSizeSpeed) * )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::PARTICLE_SIZE) + R"(;
	vec4 currentParticleColor = mix(startParticleColor, endParticleColor, normalizedAge) * mix(particleColorBySpeedStart, particleColorBySpeedEnd, normalizedColorSpeed);
	)" + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + R"( = mix(particleEmissiveColorStart, particleEmissiveColorEnd, normalizedAge);

	vec2 particleCorner = particleQuadCorners[gl_VertexID];
	float particleRotationSin = sin(particleRotation.z);
	float particleRotationCos = cos(particleRotation.z);
	vec2 rotatedCorner = vec2(
		particleCorner.x * particleRotationCos - particleCorner.y * particleRotationSin,
		particleCorner.x * particleRotationSin + particleCorner.y * particleRotationCos);

	vec2 uv = particleCorner * 0.5 + 0.5;
	vec3 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + R"( = vec3(rotatedCorner, 0.0);
)";

	if (effectiveInitializationData)
	{
		vertexShader += VS_GetUV(effectiveInitializationData);
	}
	else
	{
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = vec2(uv.x, 1.f - uv.y);\n";
	}

	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX + ";\n";
	vertexShader += "\tvec4 particleCenterWorldSpace = vec4(particlePositions[particleIndex].xyz, 1.0) * " + std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX) + ";\n";
	vertexShader += "\tvec3 cameraForward = normalize(cross(" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_RIGHT) + ", " + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_UP) + "));\n";
	vertexShader += "\tvec3 billboardWorldTangent = normalize(" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_RIGHT) + " * particleRotationCos + " + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_UP) + " * particleRotationSin);\n";
	vertexShader += "\tvec3 billboardWorldOffset = " + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_RIGHT) + " * (" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + ".x * currentParticleSize) + " + std::string(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_UP) + " * (" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + ".y * currentParticleSize);\n";
	vertexShader += "\tvec4 particleWorldPosition = vec4(particleCenterWorldSpace.xyz + billboardWorldOffset, 1.0);\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = particleWorldPosition;\n";
	if (hasWorldPositionOffset)
	{
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " = particleWorldPosition * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + " = cameraForward;\n";
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + " = billboardWorldTangent;\n";
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + " = 1.f;\n";
		vertexShader += VS_GetVertexColorText("currentParticleColor");
	}
	if (effectiveInitializationData)
	{
		vertexShader += VS_GetWorldPositionOffsetText(effectiveInitializationData, "particleWorldPosition");
	}
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = particleWorldPosition;\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + " = cameraForward;\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT) + " = billboardWorldTangent;\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN) + " = 1.f;\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + " = currentParticleColor;\n";

	vertexShader += VS_GetLightSpaceFragmentPositionCalculations();

	vertexShader += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
}
)";

	return vertexShader;
}

std::string ShaderBuilder::ParticleRenderPass_GetStaticMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetMainLayouts();
	vertexShader += VS_GetUniforms();
	vertexShader += FS_GetDirectionalLightStruct();
	vertexShader += FS_GetPointLightStruct();
	vertexShader += FS_GetSpotLightStruct();
	vertexShader += FS_GetLightArrayUniforms();
	vertexShader += VS_GetLightShadowViewMatrixUniforms();
	vertexShader += VS_GetLightOutputs();

	vertexShader += R"(
layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::POSITION) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::POSITION_BUFFER_NAME) + R"(
{
	vec4 particlePositions[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::VELOCITY) + R"() readonly buffer ParticleVelocityBuffer
{
	vec4 particleVelocities[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::COLOR) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::COLOR_BUFFER_NAME) + R"(
{
	vec4 particleColors[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::LIFETIME) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::LIFETIME_BUFFER_NAME) + R"(
{
	vec2 particleLifetimes[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::ALIVE_INDEX) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::ALIVE_INDEX_BUFFER_NAME) + R"(
{
	uint aliveParticleIndices[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::END_COLOR) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::END_COLOR_BUFFER_NAME) + R"(
{
	vec4 particleEndColors[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::SIZE) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::SIZE_BUFFER_NAME) + R"(
{
	vec2 particleSizes[];
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::Particle::ROTATION) + R"() readonly buffer )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::ROTATION_BUFFER_NAME) + R"(
{
	vec4 particleRotations[];
};

uniform float )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::PARTICLE_SIZE) + R"(;
uniform vec2 particleSizeBySpeedRange;
uniform vec2 particleSizeBySpeedValues;
uniform vec2 particleColorBySpeedRange;
uniform vec4 particleColorBySpeedStart;
uniform vec4 particleColorBySpeedEnd;
uniform vec3 particleEmissiveColorStart;
uniform vec3 particleEmissiveColorEnd;
out vec3 )" + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + R"(;
)";

	MaterialInitializationData particleInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		particleInitializationData = *initializationData;
		effectiveInitializationData = &particleInitializationData;
	}
	const bool hasWorldPositionOffset = effectiveInitializationData &&
		!effectiveInitializationData->vertexPositionOffset.result.empty();

	if (initializationData && !initializationData->vertexShaderFunctions.empty())
	{
		vertexShader += initializationData->vertexShaderFunctions;
	}

	if (initializationData && !initializationData->vertexShaderUniforms.empty())
	{
		vertexShader += initializationData->vertexShaderUniforms;
	}

	vertexShader += R"(
void main()
{
	uint particleIndex = aliveParticleIndices[gl_InstanceID];
	vec4 startParticleColor = particleColors[particleIndex];
	vec4 endParticleColor = particleEndColors[particleIndex];
	vec2 particleLifetime = particleLifetimes[particleIndex];
	vec2 particleSizeRange = particleSizes[particleIndex];
	vec3 particleRotation = particleRotations[particleIndex].xyz;
	float particleSpeed = length(particleVelocities[particleIndex].xyz);
	float normalizedAge = particleLifetime.y < 0.0 ? 0.0 : 1.0 - clamp(particleLifetime.x / max(particleLifetime.y, 0.0001), 0.0, 1.0);
	float normalizedSizeSpeed = clamp((particleSpeed - particleSizeBySpeedRange.x) / max(particleSizeBySpeedRange.y - particleSizeBySpeedRange.x, 0.0001), 0.0, 1.0);
	float normalizedColorSpeed = clamp((particleSpeed - particleColorBySpeedRange.x) / max(particleColorBySpeedRange.y - particleColorBySpeedRange.x, 0.0001), 0.0, 1.0);
	float currentParticleSize = mix(particleSizeRange.x, particleSizeRange.y, normalizedAge) * mix(particleSizeBySpeedValues.x, particleSizeBySpeedValues.y, normalizedSizeSpeed) * )" + std::string(SHADER_VARIABLE_NAMES::PARTICLE::PARTICLE_SIZE) + R"(;
	vec4 currentParticleColor = mix(startParticleColor, endParticleColor, normalizedAge) * mix(particleColorBySpeedStart, particleColorBySpeedEnd, normalizedColorSpeed);
	)" + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + R"( = mix(particleEmissiveColorStart, particleEmissiveColorEnd, normalizedAge);
	vec3 particlePosition = particlePositions[particleIndex].xyz;

	float particleRotationSinX = sin(particleRotation.x);
	float particleRotationCosX = cos(particleRotation.x);
	float particleRotationSinY = sin(particleRotation.y);
	float particleRotationCosY = cos(particleRotation.y);
	float particleRotationSinZ = sin(particleRotation.z);
	float particleRotationCosZ = cos(particleRotation.z);

	mat3 particleRotationMatrixX = mat3(
		1.0, 0.0, 0.0,
		0.0, particleRotationCosX, -particleRotationSinX,
		0.0, particleRotationSinX, particleRotationCosX);
	mat3 particleRotationMatrixY = mat3(
		particleRotationCosY, 0.0, particleRotationSinY,
		0.0, 1.0, 0.0,
		-particleRotationSinY, 0.0, particleRotationCosY);
	mat3 particleRotationMatrixZ = mat3(
		particleRotationCosZ, -particleRotationSinZ, 0.0,
		particleRotationSinZ, particleRotationCosZ, 0.0,
		0.0, 0.0, 1.0);
	mat3 particleRotationMatrix = particleRotationMatrixZ * particleRotationMatrixY * particleRotationMatrixX;
)";

	vertexShader += VS_GetPosition();
	vertexShader += R"(
	vec3 particleNormal = particleRotationMatrix * normal;
	vec3 particleTangent = particleRotationMatrix * tangent.xyz;
	mat3 particleLocalToWorldMatrix = transpose(particleRotationMatrix) * currentParticleSize;
	mat4 particleTransformMatrix = mat4(
		vec4(particleLocalToWorldMatrix[0], particlePosition.x),
		vec4(particleLocalToWorldMatrix[1], particlePosition.y),
		vec4(particleLocalToWorldMatrix[2], particlePosition.z),
		vec4(0.0, 0.0, 0.0, 1.0));
)";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = particleTransformMatrix * " + SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX + ";\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " = particleRotationMatrix * (" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " * currentParticleSize);\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " += particlePosition;\n";
	vertexShader += "\tvec4 particleWorldPosition = vec4(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + ", 1.f) * " + SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX + ";\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = particleWorldPosition;\n";
	if (hasWorldPositionOffset)
	{
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " = particleWorldPosition * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
		vertexShader += VS_GetUV(nullptr);
		vertexShader += VS_GetVertexNormalText(nullptr);
		vertexShader += VS_GetVertexColorText(
			"vec4(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::COLOR) + ".rgb * currentParticleColor.rgb, " +
			std::string(SHADER_VARIABLE_NAMES::VERTEX::COLOR) + ".a * currentParticleColor.a)");
	}
	if (effectiveInitializationData)
	{
		vertexShader += VS_GetWorldPositionOffsetText(effectiveInitializationData, "particleWorldPosition");
	}
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = particleWorldPosition;\n";
	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " = " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
	vertexShader += VS_GetLightSpaceFragmentPositionCalculations();

	if (effectiveInitializationData)
	{
		vertexShader += VS_GetUV(effectiveInitializationData);
		vertexShader += VS_GetVertexNormalText(effectiveInitializationData);
		vertexShader += VS_GetVertexTangentText();
	}
	else
	{
		vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = vec2(" + SHADER_VARIABLE_NAMES::VERTEX::UV + ".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y);\n";
		vertexShader += VS_GetVertexNormalText(nullptr);
		vertexShader += VS_GetVertexTangentText();
	}

	vertexShader += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + " = vec4(" + SHADER_VARIABLE_NAMES::VERTEX::COLOR + ".rgb * currentParticleColor.rgb, " + SHADER_VARIABLE_NAMES::VERTEX::COLOR + ".a * currentParticleColor.a);\n";
	vertexShader += "\tgl_Position = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + ";\n";
	vertexShader += "}\n";

	return vertexShader;
}

std::string ShaderBuilder::ParticleRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader) const
{
	MaterialInitializationData particleInitializationData(initializationData ? initializationData->owner : nullptr);
	if (initializationData)
	{
		particleInitializationData = *initializationData;
	}

	std::string particleBaseColorExpression;
	if (initializationData && !initializationData->baseColor.result.empty())
	{
		particleBaseColorExpression = initializationData->baseColor.result;
	}
	else if (shader)
	{
		for (const Texture* texture : GetDeterministicallySortedTextures(shader))
		{
			if (texture->GetTextureUsage() == TextureUsage::Diffuse)
			{
				particleBaseColorExpression = General_FS_GetDiffuseTextureSampling(texture);
				break;
			}
		}
	}

	if (particleBaseColorExpression.empty())
	{
		particleBaseColorExpression = SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR;
	}
	else
	{
		particleBaseColorExpression = TrimTrailingStatementTerminators(particleBaseColorExpression);
	}

	particleInitializationData.baseColor.result =
		"(" + particleBaseColorExpression + ") * " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + ";";

	std::string particleEmissiveColorExpression;
	if (initializationData && !initializationData->emissiveColor.result.empty())
	{
		particleEmissiveColorExpression = initializationData->emissiveColor.result;
	}

	if (particleEmissiveColorExpression.empty())
	{
		particleEmissiveColorExpression = SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR;
	}
	else
	{
		particleEmissiveColorExpression = TrimTrailingStatementTerminators(particleEmissiveColorExpression);
	}

	particleInitializationData.emissiveColor.result =
		"(" + particleEmissiveColorExpression + ") + " + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + ";";
	particleInitializationData.fragmentShaderUniforms +=
		"in vec3 " + std::string(PARTICLE_EMISSIVE_COLOR_VARYING_NAME) + ";\n";

	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.materialInitializationData = &particleInitializationData;
	fragmentShaderInitializationData.shader = shader;
	fragmentShaderInitializationData.renderPassType = RenderPassType::Forward;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilder::GPUFoliageRenderPass_GetMeshVertexShaderScript(MaterialInitializationData* initializationData, const Shader* shader, RenderPassType renderPassType) const
{
	MaterialInitializationData foliageInitializationData(initializationData ? initializationData->owner : nullptr);
	MaterialInitializationData* effectiveInitializationData = initializationData;
	if (initializationData)
	{
		foliageInitializationData = *initializationData;
		foliageInitializationData.boneCount = 0;
		foliageInitializationData.meshType = MeshType::Static;
		effectiveInitializationData = &foliageInitializationData;
	}

	VertexShaderInitializationData vertexShaderInitializationData;
	vertexShaderInitializationData.materialInitializationData = effectiveInitializationData;
	vertexShaderInitializationData.shader = shader;
	vertexShaderInitializationData.renderPassType = renderPassType;
	vertexShaderInitializationData.usesGPUFoliageInstanceBuffer = true;
	vertexShaderInitializationData.vertexColorExpression =
		std::string(SHADER_VARIABLE_NAMES::VERTEX::COLOR) + " * foliageInstance.color";
	return General_VS_GetScript(vertexShaderInitializationData);
}

std::string ShaderBuilder::GPUFoliageRenderPass_GetFragmentShaderScript(MaterialInitializationData* initializationData, const Shader* shader, RenderPassType renderPassType) const
{
	switch (renderPassType)
	{
	case RenderPassType::GeometryBuffer:
		return GeometryBufferPass_GetFragmentShaderScript(initializationData, shader);
	case RenderPassType::Shadow:
		return ShadowPass_GetFragmentShaderScript(initializationData, shader);
	case RenderPassType::PointLightShadow:
		return PointShadowPass_GetFragmentShaderScript(initializationData, shader);
	case RenderPassType::CubemapCapture:
		return CubemapRenderPass_GetFragmentShaderScript(initializationData, shader);
	case RenderPassType::Forward:
	default:
		return ForwardRenderPass_GetFragmentShaderScript(initializationData, shader);
	}
}

std::string ShaderBuilder::DeferredRenderPass_GetVertexShaderScript()
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

std::string ShaderBuilder::DeferredRenderPass_GetFragmentShaderScript()
{
	std::string outputVariables = FS_GetOutputVariables();
	std::string outputVariableAssignments = FS_GetOutputVariableAssignments();

	FragmentShaderInitializationData fragmentShaderInitializationData(outputVariables, outputVariableAssignments);
	fragmentShaderInitializationData.renderPassType = RenderPassType::Deferred;
	return General_FS_GetScript(fragmentShaderInitializationData);
}

std::string ShaderBuilder::FS_GetOutputVariables() const
{
	std::string output = "\n// Base Material Variables\n";

	output += "out vec4 ";
	output += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	output += ";\n\n\n";

	return output;
}

std::string ShaderBuilder::FS_GetOutputVariableAssignments() const
{
	return std::string("\t") + SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR +
		" = vec4(CalculatePBRAmbientLight() + " + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + " + " +
		SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + ", " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + ".a);";
}

std::string ShaderBuilder::FS_GetUnlitOutputVariableAssignments() const
{
	return std::string("\t") + SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR +
		" = vec4(" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR +
		", " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + ".a);";
}

std::string ShaderBuilder::GeometryBufferPass_GetOutputVariables() const
{
	std::string variables = R"(

layout(location = 0) out vec3 )" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION) + R"(;
layout(location = 1) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
layout(location = 2) out vec4 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
layout(location = 3) out vec4 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS + R"(;
layout(location = 4) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR + R"(;

)";

	return variables;
}

std::string ShaderBuilder::GeometryBufferPass_GetOutputVariableAssignments(MaterialInitializationData* initializationData) const
{
	std::string assignments = "\n";
	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION;
	assignments += " = ";
	assignments += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
	assignments += ".xyz;\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL;
	assignments += " = surfaceNormal;\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE;
	assignments += ".xyz = ";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR;
	assignments += ".xyz;\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE;
	assignments += ".w = ";
	assignments += SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID;
	assignments += ";\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS;
	assignments += " = vec4(finalAmbientOcclusion, finalMetallic, finalRoughness, ";
	assignments += MaterialUsesReflectionProbe(initializationData) ? "1.f" : "0.f";
	assignments += ");\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR;
	assignments += " = ";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR;
	assignments += ";\n";

	return assignments;
}

std::string ShaderBuilder::DeferredRenderPass_GetGBufferTextureUniforms() const
{
	return R"(
uniform sampler2D )" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION) + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS + R"(;
uniform sampler2D )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR + R"(;
)";
}

std::string ShaderBuilder::DeferredRenderPass_GetGBufferVariables() const
{
	return R"(
vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
float )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"( = 0.f;
float )" + SHADER_VARIABLE_NAMES::REFLECTION_PROBE::USAGE + R"( = 0.f;
float )" + SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID + R"( = -1;
)";
}

std::string ShaderBuilder::DeferredRenderPass_GetGBufferVariableAssignments() const
{
	std::string assignments = "\n";
	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR;
	assignments += " = texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ");\n\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
	assignments += " = vec4(texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ").xyz, 1.f);\n\n";

	assignments += "\tsurfaceNormal = normalize(texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ").xyz);\n\n";
	assignments += "\tgeometryNormal = surfaceNormal;\n\n";

	assignments += "\tvec4 ambientOcclusionMetallicRoughness = texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ");\n";
	assignments += "\tfinalAmbientOcclusion = ambientOcclusionMetallicRoughness.x;\n";
	assignments += "\tfinalMetallic = ambientOcclusionMetallicRoughness.y;\n";
	assignments += "\tfinalRoughness = max(ambientOcclusionMetallicRoughness.z, 0.04f);\n\n";
	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::REFLECTION_PROBE::USAGE;
	assignments += " = ambientOcclusionMetallicRoughness.w;\n\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR;
	assignments += " = texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ").xyz;\n";

	return assignments;
}

std::string ShaderBuilder::DeferredRenderPass_GetUnlitCheck() const
{
	return "\n\t" + std::string(SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID) + " = " +
		SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + ".a;\n" +
		FS_GetUnlitCheck(true);
}

std::string ShaderBuilder::General_FS_GetMaterialVariables(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
{
	std::string materialVariableText = "\n// Base Material Variables\n";

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

	materialVariableText += "vec4 ";
	materialVariableText += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR;
	materialVariableText += ";\n";

	materialVariableText += "vec3 ";
	materialVariableText += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR;
	materialVariableText += ";\n";

	materialVariableText += "float finalAmbientOcclusion;\n";
	materialVariableText += "float finalMetallic;\n";
	materialVariableText += "float finalRoughness;\n";
	materialVariableText += "vec3 geometryNormal;\n";
	materialVariableText += "vec3 geometryTangent;\n";
	materialVariableText += "vec3 geometryBitangent;\n";
	materialVariableText += "vec3 surfaceNormal;\n";

	bool requiresAlphaTest = fragmentShaderInitializationData.materialInitializationData &&
		fragmentShaderInitializationData.materialInitializationData->owner &&
		(fragmentShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			fragmentShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	bool includeMaterialVariables = fragmentShaderInitializationData.renderPassType == RenderPassType::Forward ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::GeometryBuffer ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::CubemapCapture ||
		(requiresAlphaTest && (fragmentShaderInitializationData.renderPassType == RenderPassType::Shadow || fragmentShaderInitializationData.renderPassType == RenderPassType::PointLightShadow));

	if (includeMaterialVariables)
	{
		materialVariableText += "in vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
		materialVariableText += ";\n";

		materialVariableText += "uniform vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION;
		materialVariableText += ";\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::METALLIC;
		materialVariableText += ";\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS;
		materialVariableText += ";\n";

		materialVariableText += "uniform int ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID;
		materialVariableText += ";\n";

		materialVariableText += "uniform vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "in vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
		materialVariableText += ";\n";

		materialVariableText += "in vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT;
		materialVariableText += ";\n";

		materialVariableText += "in float ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN;
		materialVariableText += ";\n";

		materialVariableText += "in vec4 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "uniform float ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY;
		materialVariableText += ";\n\n";
	}

	return materialVariableText;
}

std::string ShaderBuilder::FS_GetLightSpaceFragmentPositions(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
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

std::string ShaderBuilder::FS_GetDirectionalLightColorFunction() const
{
	return R"(
vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	return CalculatePBRLighting(normalize(-direction), intensity);
}
)";
}

std::string ShaderBuilder::FS_GetPointLightColorFunction() const
{
	return R"(
vec3 CalculatePointLightColor(vec3 position, vec3 intensity, float radius)
{
	vec3 lightDirection = position - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"();
	float lightDistanceSquared = dot(lightDirection, lightDirection);
	float radiusSquared = radius * radius;

	if(radiusSquared < lightDistanceSquared || lightDistanceSquared <= 0.f)
	{
		return vec3(0.f);
	}

	float lightDistance = sqrt(lightDistanceSquared);
	lightDirection /= lightDistance;
	vec3 radiance = intensity / max(lightDistanceSquared, 0.0001f);
	return CalculatePBRLighting(lightDirection, radiance);
}
)";
}

std::string ShaderBuilder::FS_GetSpotLightColorFunction() const
{
	return R"(
vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	float lightMultiplier = 0.f;

	vec3 lightDirection = position - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"();
	float lightDistance = length(lightDirection);

	if(lightDistance <= 0.f)
	{
		return vec3(0.f);
	}

	lightDirection /= lightDistance;
	vec3 lightToFragmentDirection = -lightDirection;

	float cosCoverage = cos(coverageAngle);
	float cosFalloff = cos(falloffAngle);
	float cosTheta = dot(lightToFragmentDirection, direction);

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

	vec3 radiance = (intensity / (lightDistance * lightDistance)) * lightMultiplier;
	return CalculatePBRLighting(lightDirection, radiance);
}
)";
}

std::string ShaderBuilder::FS_GetDirectionalLightStruct() const
{
	std::string directionalLightStruct =
		R"(
struct )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_STRUCT_NAME) + R"(
{
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
	int )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
};
)";

	return directionalLightStruct;
}

std::string ShaderBuilder::FS_GetPointLightStruct() const
{
	std::string pointLightStruct =
		R"(
struct )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_STRUCT_NAME) + R"(
{
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"( ;
	vec3 )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(;
	int )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
	float padding0;
	float padding1;
	float padding2;
};

)";
	return pointLightStruct;
}

std::string ShaderBuilder::FS_GetSpotLightStruct() const
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
	int )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
	float padding0;
	float padding1;
	float padding2;
};
)";
	return spotLightStruct;
}

std::string ShaderBuilder::FS_GetLightArrayUniforms() const
{
	return R"(
layout (std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::LIGHT_DATA) + R"() readonly buffer LightDataBuffer
{
	)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
	)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
	)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_STRUCT_NAME + " " + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
};

uniform int )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE) + R"(;
uniform int )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE) + R"(;
uniform int )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE) + R"(;
)";
}

std::string ShaderBuilder::FS_GetShadowMapUniforms() const
{
	return R"(
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
uniform samplerCube )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
)";
}

std::string ShaderBuilder::FS_GetReflectionProbeUniforms() const
{
	return R"(
uniform bool )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE) + R"(;
uniform samplerCube )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP) + R"(;
uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::POSITION) + R"(;
uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MIN) + R"(;
uniform vec3 )" + std::string(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MAX) + R"(;
)";
}

void ShaderBuilder::ApplyTextureBackedMaterialDefaults(MaterialInitializationData* initializationData, const Shader* shader) const
{
	if (!initializationData || !shader)
	{
		return;
	}

	for (const Texture* texture : GetDeterministicallySortedTextures(shader))
	{
		switch (texture->GetTextureUsage())
		{
		case TextureUsage::Diffuse:
			if (initializationData->baseColor.result.empty())
			{
				initializationData->baseColor.result = General_FS_GetDiffuseTextureSampling(texture);
			}
			break;
		case TextureUsage::Emissive:
			if (initializationData->emissiveColor.result.empty())
			{
				initializationData->emissiveColor.result = General_FS_GetEmissiveTextureSampling(texture);
			}
			break;
		case TextureUsage::AmbientOcclusion:
			if (initializationData->ambientOcclusion.result.empty())
			{
				initializationData->ambientOcclusion.result = General_FS_GetScalarTextureSampling(texture);
			}
			break;
		case TextureUsage::Metallic:
			if (initializationData->metallic.result.empty())
			{
				initializationData->metallic.result = General_FS_GetScalarTextureSampling(texture);
			}
			break;
		case TextureUsage::Roughness:
			if (initializationData->roughness.result.empty())
			{
				initializationData->roughness.result = General_FS_GetScalarTextureSampling(texture);
			}
			break;
		case TextureUsage::Normal:
			if (initializationData->fragmentNormal.result.empty())
			{
				initializationData->fragmentNormal.result = General_FS_GetNormalTextureSampling(texture);
				initializationData->fragmentNormalIsTangentSpace = true;
			}
			break;
		case TextureUsage::ORM:
			if (initializationData->ambientOcclusion.result.empty() && initializationData->roughness.result.empty() && initializationData->metallic.result.empty())
			{
				General_FS_GetORMTextureSampling(texture, initializationData);
			}
			break;
		default:
			break;
		}
	}
}

std::string ShaderBuilder::General_FS_GetShaderTextureUniforms(const MaterialInitializationData* initializationData, const Shader* shader) const
{
	(void)initializationData;
	if (!shader)
	{
		return "";
	}

	std::string uniforms = "";
	std::unordered_set<std::string> addedTextureNames;

	for (const Texture* texture : GetDeterministicallySortedTextures(shader))
	{
		const std::string textureUniformName = texture->GetShaderUniformName();
		if (addedTextureNames.find(textureUniformName) == addedTextureNames.end())
		{
			uniforms += "uniform sampler2D " + textureUniformName + ";\n";
			// Standalone textures use the identity transform, which keeps atlas fallback shaders valid.
			uniforms += "uniform vec4 " + texture->GetAtlasUVTransformUniformName() + ";\n";
			addedTextureNames.insert(textureUniformName);
		}
	}

	return uniforms;
}

std::string ShaderBuilder::General_FS_GetTextureUVExpression(const Texture* texture) const
{
	if (texture && (texture->GetUsesAtlasTexture() || texture->GetWaitsForTextureAtlas()))
	{
		const std::string baseUV = SHADER_VARIABLE_NAMES::TEXTURE::UV;
		const std::string wrappedUV =
			"vec2(" +
			GetTextureAtlasWrappedUVComponent(baseUV, 'x', texture->GetTextureWrappingS()) +
			", " +
			GetTextureAtlasWrappedUVComponent(baseUV, 'y', texture->GetTextureWrappingT()) +
			")";

		return "(" + wrappedUV + " * " + texture->GetAtlasUVTransformUniformName() + ".xy + " + texture->GetAtlasUVTransformUniformName() + ".zw)";
	}

	return SHADER_VARIABLE_NAMES::TEXTURE::UV;
}

std::string ShaderBuilder::General_FS_GetDiffuseTextureSampling(const Texture* texture) const
{
	return texture ? std::string("texture(" + texture->GetShaderUniformName() + ", " + General_FS_GetTextureUVExpression(texture) + "); ") : "";
}

std::string ShaderBuilder::General_FS_GetScalarTextureSampling(const Texture* texture) const
{
	return texture ? std::string("texture(" + texture->GetShaderUniformName() + ", " + General_FS_GetTextureUVExpression(texture) + ").r; ") : "";
}

std::string ShaderBuilder::General_FS_GetNormalTextureSampling(const Texture* texture) const
{
	return texture ? std::string("normalize(texture(" + texture->GetShaderUniformName() + ", " + General_FS_GetTextureUVExpression(texture) + ").xyz * 2.f - vec3(1.f)); ") : "";
}

std::string ShaderBuilder::General_FS_GetEmissiveTextureSampling(const Texture* texture) const
{
	return texture ? std::string("texture(" + texture->GetShaderUniformName() + ", " + General_FS_GetTextureUVExpression(texture) + ").xyz; ") : "";
}

void ShaderBuilder::General_FS_GetORMTextureSampling(const Texture* texture, MaterialInitializationData* initializationData) const
{
	if (texture)
	{
		initializationData->ambientOcclusion.calculation = std::string("vec3 ORMValue = ") + "texture(" + texture->GetShaderUniformName() + ", " + General_FS_GetTextureUVExpression(texture) + ").xyz;";
		initializationData->ambientOcclusion.result = "ORMValue.x";
		initializationData->roughness.result = "ORMValue.y";
		initializationData->metallic.result = "ORMValue.z";
	}
}

std::string ShaderBuilder::General_FS_GetDiffuseTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + "); ");
}

std::string ShaderBuilder::General_FS_GetScalarTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").r; ");
}

std::string ShaderBuilder::General_FS_GetNormalTextureSampling(const std::string& textureName) const
{
	return std::string("normalize(texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").xyz * 2.f - vec3(1.f)); ");
}

std::string ShaderBuilder::General_FS_GetEmissiveTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").xyz; ");
}

std::string ShaderBuilder::FS_GetLightCalculationIterators(bool includeShadowing) const
{
	if (!includeShadowing)
	{
		return R"(
	for(int directionalLightIndex = 0; directionalLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++directionalLightIndex)
	{
		)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateDirectionalLightColor(
			)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
	}

	for(int pointLightIndex = 0; pointLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++pointLightIndex)
	{
		)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculatePointLightColor(
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
	}

	for(int spotLightIndex = 0; spotLightIndex < )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE + R"(; ++spotLightIndex)
	{
		)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + R"( += CalculateSpotLightColor(
			)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
	}
)";
	}

	return R"(
	for(int directionalLightIndex = 0; directionalLightIndex < )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE) + R"(; ++directionalLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_ARRAY_NAME + "[directionalLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"( != 0)
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
		vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + " = CalculatePointLightColor(" + R"(
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(,
			)" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();

		if()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"( != 0)
		{
			vec3 )" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"(.xyz + geometryNormal * 0.025f - )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
			float pointLightRadius = max()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"(, 0.0001f);
			float currentNormalizedDepth = length()" + std::string(SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR) + R"() / pointLightRadius;
			float closestNormalizedDepth = texture()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME + "[pointLightIndex], " + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"().r;
			float shadowVisibility = currentNormalizedDepth - 0.0025f <= closestNormalizedDepth ? 1.f : clamp()" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(, 0.f, 1.f);
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + " += shadowVisibility * " + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
		}
		else
		{
			)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY + " += " + SHADER_VARIABLE_NAMES::LIGHT::CURRENT_LIGHT_INTENSITY + R"(;
		}
	}

	for(int spotLightIndex = 0; spotLightIndex < )" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE + R"(; ++spotLightIndex)
	{
		if()" + SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_ARRAY_NAME + "[spotLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"( != 0)
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

std::string ShaderBuilder::FS_InitializeBaseColor(MaterialInitializationData* initializationData, bool discardTransparent) const
{
	std::string result = "";

	if (initializationData && !initializationData->baseColor.calculation.empty())
	{
		result += initializationData->baseColor.calculation + "\n";
	}

	result += std::string("\n\t") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + " = ";

	if (initializationData && !initializationData->baseColor.result.empty())
	{
		result += initializationData->baseColor.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR) + ";";
	}

	if (initializationData &&
		initializationData->owner &&
		(initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			(discardTransparent && initializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent)))
	{
		result += "\tif (" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR) + ".a < 0.5f) discard;\n";
	}

	return result;
}

std::string ShaderBuilder::FS_InitializeEmissiveColor(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->emissiveColor.calculation.empty())
	{
		result += initializationData->emissiveColor.calculation + "\n";
	}

	result += std::string("\n\t") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + " = ";

	if (initializationData && !initializationData->emissiveColor.result.empty())
	{
		result += initializationData->emissiveColor.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR) + ";";
	}

	return result;
}

std::string ShaderBuilder::FS_InitializeAmbientOcclusion(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->ambientOcclusion.calculation.empty())
	{
		result += initializationData->ambientOcclusion.calculation + "\n";
	}

	result += "\n\tfinalAmbientOcclusion = ";

	if (initializationData && !initializationData->ambientOcclusion.result.empty())
	{
		result += initializationData->ambientOcclusion.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION) + ";";
	}

	result += "\tfinalAmbientOcclusion = clamp(finalAmbientOcclusion, 0.f, 1.f);\n";
	return result;
}

std::string ShaderBuilder::FS_InitializeMetallic(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->metallic.calculation.empty())
	{
		result += initializationData->metallic.calculation + "\n";
	}

	result += "\n\tfinalMetallic = ";

	if (initializationData && !initializationData->metallic.result.empty())
	{
		result += initializationData->metallic.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::METALLIC) + ";";
	}

	result += "\tfinalMetallic = clamp(finalMetallic, 0.f, 1.f);\n";
	return result;
}

std::string ShaderBuilder::FS_InitializeRoughness(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->roughness.calculation.empty())
	{
		result += initializationData->roughness.calculation + "\n";
	}

	result += "\n\tfinalRoughness = ";

	if (initializationData && !initializationData->roughness.result.empty())
	{
		result += initializationData->roughness.result;
	}
	else
	{
		result += std::string(SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS) + ";";
	}

	result += "\tfinalRoughness = clamp(finalRoughness, 0.04f, 1.f);\n";
	return result;
}

std::string ShaderBuilder::FS_InitializeSurfaceNormal(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	result += "\n\tgeometryNormal = normalize(";
	result += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	result += ");\n";

	result += "\tgeometryTangent = ";
	result += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT;
	result += " - geometryNormal * dot(geometryNormal, ";
	result += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT;
	result += ");\n";
	result += "\tif (dot(geometryTangent, geometryTangent) < 0.000001f)\n";
	result += "\t{\n";
	result += "\t\tvec3 tangentFallbackAxis = abs(geometryNormal.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);\n";
	result += "\t\tgeometryTangent = cross(tangentFallbackAxis, geometryNormal);\n";
	result += "\t}\n";
	result += "\tgeometryTangent = normalize(geometryTangent);\n";
	result += "\tgeometryBitangent = normalize(cross(geometryNormal, geometryTangent)) * ";
	result += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN;
	result += ";\n";

	if (initializationData && !initializationData->fragmentNormal.calculation.empty())
	{
		result += initializationData->fragmentNormal.calculation + "\n";
	}

	if (initializationData && !initializationData->fragmentNormal.result.empty())
	{
		result += "\n\tvec3 fragmentNormalResult = normalize(";
		result += TrimTrailingStatementTerminators(initializationData->fragmentNormal.result);
		result += ");\n";

		if (initializationData->fragmentNormalIsTangentSpace)
		{
			result += "\tsurfaceNormal = normalize(\n";
			result += "\t\tfragmentNormalResult.x * geometryTangent +\n";
			result += "\t\tfragmentNormalResult.y * geometryBitangent +\n";
			result += "\t\tfragmentNormalResult.z * geometryNormal);\n";
		}
		else
		{
			result += "\tsurfaceNormal = fragmentNormalResult;\n";
		}
	}
	else
	{
		result += "\tsurfaceNormal = geometryNormal;\n";
	}

	result += "\tsurfaceNormal = normalize(surfaceNormal);\n";
	return result;
}

std::string ShaderBuilder::FS_GetUnlitColorExpression(bool includeReflectionProbe) const
{
	std::string colorExpression = SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR;
	if (includeReflectionProbe)
	{
		colorExpression = "(CalculateReflectionProbeSpecularAmbient() * finalAmbientOcclusion) + " +
			colorExpression;
	}

	return colorExpression;
}

std::string ShaderBuilder::FS_GetUnlitCheck(bool includeReflectionProbe) const
{
	return R"(
	if ()" + std::string(SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID) + " == " + std::to_string((int)MaterialShadingType::Unlit) + R"()
	{
		)" + SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR + " = vec4(" + FS_GetUnlitColorExpression(includeReflectionProbe) + ", " + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.a);
		return;
	}
)";
}

std::string ShaderBuilder::FS_GetPBRFunctions(bool includeReflectionProbe) const
{
	std::string reflectionProbeFunction;
	std::string reflectionProbeAmbientExpression = "vec3(0.f)";

	if (includeReflectionProbe)
	{
		reflectionProbeAmbientExpression = "CalculateReflectionProbeSpecularAmbient()";

		reflectionProbeFunction =
			std::string(R"(
float GetSafeReflectionProbeRayComponent(float component)
{
    if(abs(component) < 0.0001f)
    {
        return component < 0.f ? -0.0001f : 0.0001f;
    }

    return component;
}

vec3 GetBoxProjectedReflectionProbeDirection(vec3 fragmentPosition, vec3 reflectionDirection)
{
    vec3 rayDirection = normalize(reflectionDirection);
    rayDirection = vec3(
        GetSafeReflectionProbeRayComponent(rayDirection.x),
        GetSafeReflectionProbeRayComponent(rayDirection.y),
        GetSafeReflectionProbeRayComponent(rayDirection.z));
    vec3 firstPlaneIntersection = ()") +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MIN +
			R"( - fragmentPosition) / rayDirection;
    vec3 secondPlaneIntersection = ()" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::BOX_MAX +
			R"( - fragmentPosition) / rayDirection;
    vec3 furthestPlaneIntersection = max(firstPlaneIntersection, secondPlaneIntersection);
    float intersectionDistance = min(min(furthestPlaneIntersection.x, furthestPlaneIntersection.y), furthestPlaneIntersection.z);

    if(intersectionDistance <= 0.f)
    {
        return reflectionDirection;
    }

    vec3 boxIntersection = fragmentPosition + rayDirection * intersectionDistance;
    return boxIntersection - )" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::POSITION +
			R"(;
}

vec3 CalculateReflectionProbeSpecularAmbient()
{
    if(0.f < )" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::USAGE +
			R"( && )" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE +
			R"()
    {
        vec3 viewDirection = pbrViewDirection;
        vec3 fresnel = FresnelSchlick(max(dot(surfaceNormal, viewDirection), 0.f), pbrF0);
        vec3 reflectionDirection = reflect(-viewDirection, surfaceNormal);
        reflectionDirection = GetBoxProjectedReflectionProbeDirection(vec3()" +
			SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE +
			R"(), reflectionDirection);
        float maxMipLevel = max(float(textureQueryLevels()" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP +
			R"() - 1), 0.f);
        float mipLevel = finalRoughness * maxMipLevel;
        return textureLod()" +
			SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP +
			R"(, reflectionDirection, mipLevel).rgb * fresnel;
    }

    return vec3(0.f);
}
)";
	}

	return R"(
const float PI = 3.14159265359f;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (vec3(1.f) - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

float DistributionGGX(vec3 normal, vec3 halfVector, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float normalDotHalf = max(dot(normal, halfVector), 0.f);
    float normalDotHalf2 = normalDotHalf * normalDotHalf;

    float denominator = normalDotHalf2 * (a2 - 1.f) + 1.f;
    return a2 / max(PI * denominator * denominator, 0.0001f);
}

float GeometrySchlickGGX(float normalDotDirection, float roughness)
{
    float r = roughness + 1.f;
    float k = (r * r) / 8.f;

    return normalDotDirection / max(normalDotDirection * (1.f - k) + k, 0.0001f);
}

float GeometrySmith(vec3 normal, vec3 viewDirection, vec3 lightDirection, float roughness)
{
    float normalDotView = max(dot(normal, viewDirection), 0.f);
    float normalDotLight = max(dot(normal, lightDirection), 0.f);

    float ggxView = GeometrySchlickGGX(normalDotView, roughness);
    float ggxLight = GeometrySchlickGGX(normalDotLight, roughness);

    return ggxView * ggxLight;
}

vec3 pbrViewDirection;
vec3 pbrF0;

void InitializePBRSharedInputs()
{
    pbrViewDirection = normalize()" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION) + R"( - vec3()" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"());
    pbrF0 = mix(vec3(0.04f), )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb, finalMetallic);
}

vec3 CalculatePBRLighting(vec3 lightDirection, vec3 radiance)
{
    vec3 normal = surfaceNormal;
    vec3 viewDirection = pbrViewDirection;

    float normalDotLight = dot(normal, lightDirection);
    bool isBackface = normalDotLight < 0.f;

    if(isBackface)
    {
        if()" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"( <= 0.f)
        {
            return vec3(0.f);
        }
        normal = -normal;
        normalDotLight = -normalDotLight;
    }

    float normalDotView = max(dot(normal, viewDirection), 0.0001f);
    float clampedNormalDotLight = max(normalDotLight, 0.f);
    if(clampedNormalDotLight <= 0.f)
    {
        return vec3(0.f);
    }

    vec3 F0 = pbrF0;
    vec3 specular = vec3(0.f);
    vec3 specularEnergy = vec3(0.f);

    // Only calculate specular reflections for front-facing light
    if(!isBackface)
    {
        vec3 halfVector = normalize(viewDirection + lightDirection);
        specularEnergy = FresnelSchlick(max(dot(halfVector, viewDirection), 0.f), F0);

        float NDF = DistributionGGX(normal, halfVector, finalRoughness);
        float G = GeometrySmith(normal, viewDirection, lightDirection, finalRoughness);

        vec3 numerator = NDF * G * specularEnergy;
        float denominator = max(4.f * normalDotView * clampedNormalDotLight, 0.0001f);
        specular = numerator / denominator;
    }

    vec3 diffuseEnergy = (vec3(1.f) - specularEnergy) * (1.f - finalMetallic);

    // FIX: Divided base color by PI for energy conservation
    vec3 directLighting = ((diffuseEnergy * )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb / PI) + specular) * radiance * clampedNormalDotLight;

    if(isBackface)
    {
        directLighting *= )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
    }

    return directLighting;
}
)" + reflectionProbeFunction + R"(
vec3 CalculatePBRAmbientLight()
{
    vec3 viewDirection = pbrViewDirection;
    vec3 F0 = pbrF0;
    vec3 fresnel = FresnelSchlick(max(dot(surfaceNormal, viewDirection), 0.f), F0);
    vec3 diffuseAmbient = 0.12f * )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb * finalAmbientOcclusion;
    vec3 specularAmbient = )" + reflectionProbeAmbientExpression + R"(;
    vec3 diffuseMultiplier = (vec3(1.f) - fresnel) * (1.f - finalMetallic);
    return diffuseAmbient * diffuseMultiplier + specularAmbient * finalAmbientOcclusion;
}
)";
}

std::string ShaderBuilder::VS_GetMainLayouts() const
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

	layouts += "layout(location = 4) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::TANGENT;
	layouts += ";\n";

	return layouts;
}

std::string ShaderBuilder::VS_GetInstancedStaticMeshLayouts() const
{
	std::string layouts = "\n\n";

	layouts += "layout(location = 5) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_0;
	layouts += ";\n";

	layouts += "layout(location = 6) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_1;
	layouts += ";\n";

	layouts += "layout(location = 7) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_2;
	layouts += ";\n";

	layouts += "layout(location = 8) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_3;
	layouts += ";\n";

	return layouts;
}

std::string ShaderBuilder::VS_GetSkeletalMeshLayouts() const
{
	std::string layouts = "\n\n";

	layouts += "layout(location = 5) in ivec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::BONE_IDS;
	layouts += ";\n";

	layouts += "layout(location = 6) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::WEIGHTS;
	layouts += ";\n";

	return layouts;
}


std::string ShaderBuilder::VS_GetSkeletalMeshVariables() const
{
	std::string variables = "\n\n";

	variables += "#define ";
	variables += SHADER_VARIABLE_NAMES::SKELETAL_MESH::MAX_BONE_SIZE_MACRO;
	variables += " ";
	variables += std::to_string(MAX_BONE_SIZE_PER_VERTEX);
	variables += "\n";

	return variables;
}

std::string ShaderBuilder::VS_GetSkeletalMeshUniforms(int boneCount) const
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

std::string ShaderBuilder::VS_GetInstancedStaticMeshTransformationMatrixCalculation() const
{
	return std::string("\tmat4 ") + SHADER_VARIABLE_NAMES::POSITIONING::INSTANCE_TRANSFORMATION_MATRIX + " = mat4(" +
		SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_0 + ", " +
		SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_1 + ", " +
		SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_2 + ", " +
		SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_3 + ");\n";
}

std::string ShaderBuilder::VS_GetGPUFoliageInstanceBuffer() const
{
	return R"(
struct GPUFoliageInstanceData
{
	mat4 transform;
	vec4 color;
};

layout(std430, binding = )" + std::to_string(ShaderBindingPoints::ShaderStorage::INSTANCE_DATA) + R"() readonly buffer GPUFoliageInstanceBuffer
{
	GPUFoliageInstanceData foliageInstances[];
};
)";
}

std::string ShaderBuilder::VS_GetUniforms() const
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

	uniforms += "out vec3 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT;
	uniforms += ";\n";

	uniforms += "out float ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN;
	uniforms += ";\n";

	uniforms += "out vec4 ";
	uniforms += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
	uniforms += ";\n";

	uniforms += "out vec2 ";
	uniforms += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	uniforms += ";\n";

	return uniforms;
}

std::string ShaderBuilder::VS_GetLightShadowViewMatrixUniforms() const
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

std::string ShaderBuilder::VS_GetLightOutputs() const
{
	return R"(

out vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
out vec4 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;

)";
}

std::string ShaderBuilder::VS_GetSkeletalMeshWeightCalculation() const
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

std::string ShaderBuilder::VS_GetMain(const VertexShaderInitializationData& vertexShaderInitializationData, const std::string& vertexShaderModelMatrixVariable) const
{
	std::string vsMain = VS_GetPosition();
	const bool hasWorldPositionOffset = vertexShaderInitializationData.materialInitializationData &&
		!vertexShaderInitializationData.materialInitializationData->vertexPositionOffset.result.empty();

	vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable + ";\n";
	vsMain += "\tvec4 materialWorldPosition = vec4(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + ", 1.f) * " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";\n";
	vsMain += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = materialWorldPosition;\n";
	if (hasWorldPositionOffset)
	{
		vsMain += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " = materialWorldPosition * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
		vsMain += VS_GetUV(nullptr);
		vsMain += VS_GetVertexNormalText(nullptr);
		vsMain += VS_GetVertexColorText(vertexShaderInitializationData.vertexColorExpression);
	}
	vsMain += VS_GetWorldPositionOffsetText(vertexShaderInitializationData.materialInitializationData, "materialWorldPosition");
	vsMain += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + " = materialWorldPosition;\n";

	bool isPointLightShadowPass = vertexShaderInitializationData.renderPassType == RenderPassType::PointLightShadow;
	bool isCubemapRenderPass = vertexShaderInitializationData.renderPassType == RenderPassType::CubemapCapture;
	bool requiresAlphaTest = vertexShaderInitializationData.materialInitializationData &&
		vertexShaderInitializationData.materialInitializationData->owner &&
		(vertexShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
			vertexShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	if ((!isPointLightShadowPass && !isCubemapRenderPass) || requiresAlphaTest)
	{
		vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
	}

	if (vertexShaderInitializationData.renderPassType == RenderPassType::Forward)
	{
		vsMain += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (!isPointLightShadowPass || requiresAlphaTest || isCubemapRenderPass)
	{
		vsMain += VS_GetUV(vertexShaderInitializationData.materialInitializationData);
		vsMain += VS_GetVertexNormalText(vertexShaderInitializationData.materialInitializationData);
		vsMain += VS_GetVertexTangentText();
		vsMain += VS_GetVertexColorText(vertexShaderInitializationData.vertexColorExpression);

		if (isPointLightShadowPass || isCubemapRenderPass)
		{
			vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
	)";
		}
		else
		{
			vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
	)";
		}
	}
	else
	{
		vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
	)";
	}

	return vsMain;
}

std::string ShaderBuilder::VS_GetPosition() const
{
	return "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " = " + std::string(SHADER_VARIABLE_NAMES::VERTEX::POSITION) + ";\n";
}

std::string ShaderBuilder::VS_GetWorldPositionOffsetText(MaterialInitializationData* initializationData, const std::string& worldPositionVariableName) const
{
	std::string result = "";

	if (!initializationData)
	{
		return result;
	}

	if (!initializationData->vertexPositionOffset.calculation.empty())
	{
		result += initializationData->vertexPositionOffset.calculation + "\n";
	}

	if (!initializationData->vertexPositionOffset.result.empty())
	{
		result += "\t// World Position Offset is applied after final world transform; provide Vertex Normal when it changes surface slope.\n";
		result += "\tvec3 materialWorldPositionOffset = " + TrimTrailingStatementTerminators(initializationData->vertexPositionOffset.result) + ";\n";
		result += "\t" + worldPositionVariableName + ".xyz += materialWorldPositionOffset;\n";
	}

	return result;
}

std::string ShaderBuilder::VS_GetUV(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (!initializationData)
	{
		return std::string("\t") + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = vec2(" + SHADER_VARIABLE_NAMES::VERTEX::UV + ".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y); \n";
	}

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

std::string ShaderBuilder::VS_GetLightSpaceFragmentPositionCalculations() const
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

std::string ShaderBuilder::VS_GetVertexNormalText(MaterialInitializationData* initializationData) const
{
	std::string vertexNormalText = "";

	if (!initializationData)
	{
		initializationData = nullptr;
	}

	if (initializationData && !initializationData->vertexNormal.calculation.empty())
	{
		vertexNormalText += initializationData->vertexNormal.calculation + "\n";
	}

	vertexNormalText += "\n\t";
	vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexNormalText += " = normalize(";

	if (initializationData && !initializationData->vertexNormal.result.empty())
	{
		vertexNormalText += TrimTrailingStatementTerminators(initializationData->vertexNormal.result);
	}
	else
	{
		vertexNormalText += SHADER_VARIABLE_NAMES::VERTEX::NORMAL;
	}

	vertexNormalText += " * transpose(inverse(mat3(" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + "))));\n";

	return vertexNormalText;
}

std::string ShaderBuilder::VS_GetVertexTangentText(const std::string& tangentExpression, const std::string& tangentSignExpression) const
{
	const std::string tangentSource = tangentExpression.empty() ? std::string(SHADER_VARIABLE_NAMES::VERTEX::TANGENT) + ".xyz" : tangentExpression;
	const std::string tangentSignSource = tangentSignExpression.empty() ? std::string(SHADER_VARIABLE_NAMES::VERTEX::TANGENT) + ".w" : tangentSignExpression;

	std::string vertexTangentText = "";
	vertexTangentText += "\n\tvec3 worldNormalForTangent = normalize(";
	vertexTangentText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
	vertexTangentText += ");\n";
	vertexTangentText += "\tvec3 worldTangent = (";
	vertexTangentText += tangentSource;
	vertexTangentText += ") * transpose(inverse(mat3(";
	vertexTangentText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX;
	vertexTangentText += ")));\n";
	vertexTangentText += "\tworldTangent = worldTangent - worldNormalForTangent * dot(worldNormalForTangent, worldTangent);\n";
	vertexTangentText += "\tif (dot(worldTangent, worldTangent) < 0.000001f)\n";
	vertexTangentText += "\t{\n";
	vertexTangentText += "\t\tvec3 tangentFallbackAxis = abs(worldNormalForTangent.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);\n";
	vertexTangentText += "\t\tworldTangent = cross(tangentFallbackAxis, worldNormalForTangent);\n";
	vertexTangentText += "\t}\n";
	vertexTangentText += "\t";
	vertexTangentText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT;
	vertexTangentText += " = normalize(worldTangent);\n";
	vertexTangentText += "\t";
	vertexTangentText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_TANGENT_SIGN;
	vertexTangentText += " = ";
	vertexTangentText += tangentSignSource;
	vertexTangentText += " < 0.f ? -1.f : 1.f;\n";

	return vertexTangentText;
}

std::string ShaderBuilder::VS_GetVertexColorText(const std::string& colorExpression) const
{
	std::string vertexColorText = "";
	const std::string resolvedColorExpression = colorExpression.empty() ? SHADER_VARIABLE_NAMES::VERTEX::COLOR : colorExpression;

	vertexColorText += "\n\t";
	vertexColorText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
	vertexColorText += " = ";
	vertexColorText += resolvedColorExpression;
	vertexColorText += ";\n";

	return vertexColorText;
}
