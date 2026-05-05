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
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include <unordered_set>

ShaderBuilder* ShaderBuilder::instance_ = nullptr;

ShaderBuilder::~ShaderBuilder()
{
}

std::string ShaderBuilder::General_FS_GetScript(const FragmentShaderInitializationData& fragmentShaderInitializationData) const
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

		fragmentShader += FS_GetPBRFunctions();
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
		fragmentShader += FS_InitializeAmbientOcclusion(fragmentShaderInitializationData.materialInitializationData);
		fragmentShader += FS_InitializeMetallic(fragmentShaderInitializationData.materialInitializationData);
		fragmentShader += FS_InitializeRoughness(fragmentShaderInitializationData.materialInitializationData);
		fragmentShader += FS_InitializeSurfaceNormal(fragmentShaderInitializationData.materialInitializationData);
	}
	else if(fragmentShaderInitializationData.renderPassType == RenderPassType::Deferred)
	{
		fragmentShader += DeferredRenderPass_GetGBufferVariableAssignments();
		fragmentShader += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (includeLightOperations)
	{
		fragmentShader += "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_INTENSITY) + " = vec3(0.f);\n";
		fragmentShader += FS_GetLightCalculationIterators();
	}
	fragmentShader += fragmentShaderInitializationData.outputVariableAssignments;
	fragmentShader += R"(
})";

	return fragmentShader;
}

std::string ShaderBuilder::General_VS_GetScript(const VertexShaderInitializationData& vertexShaderInitializationData) const
{
	std::string vertexShader = "#version " + shaderVersion_ + "\n\n";
	vertexShader += VS_GetMainLayouts();

	std::string vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX);
	const bool isInstancedStaticMesh = vertexShaderInitializationData.meshType == MeshType::InstancedStatic;

	if (isInstancedStaticMesh)
	{
		vertexShader += VS_GetInstancedStaticMeshLayouts();
		vertexShaderModelMatrixVariable = std::string(SHADER_VARIABLE_NAMES::POSITIONING::INSTANCE_TRANSFORMATION_MATRIX) + " * " + vertexShaderModelMatrixVariable;
	}

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
	if (isInstancedStaticMesh)
	{
		vertexShader += VS_GetInstancedStaticMeshTransformationMatrixCalculation();
	}
	vertexShader += VS_GetMain(vertexShaderInitializationData, vertexShaderModelMatrixVariable);
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
	std::string outputVariableAssignments = GeometryBufferPass_GetOutputVariableAssignments();

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
	bool requiresAlphaTest = initializationData && 
		(initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
		 initializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	if (requiresAlphaTest)
	{
		FragmentShaderInitializationData fragmentShaderInitializationData("", "");
		fragmentShaderInitializationData.materialInitializationData = initializationData;
		fragmentShaderInitializationData.shader = shader;
		fragmentShaderInitializationData.renderPassType = RenderPassType::Shadow;

		std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
		shadowPassFragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
		shadowPassFragmentShader += General_FS_GetShaderTextureUniforms(initializationData, shader);

		if (!initializationData->fragmentShaderFunctions.empty())
		{
			shadowPassFragmentShader += initializationData->fragmentShaderFunctions;
		}

		if (!initializationData->fragmentShaderUniforms.empty())
		{
			shadowPassFragmentShader += initializationData->fragmentShaderUniforms;
		}

		shadowPassFragmentShader += R"(
void main()
{
)";
		shadowPassFragmentShader += FS_InitializeBaseColor(initializationData, true);
		shadowPassFragmentShader += FS_InitializeEmmisiveColor(initializationData);
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
		pointShadowPassGeometryShader += "in vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "[];\n";
		pointShadowPassGeometryShader += "out vec2 " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS;\n";

		pointShadowPassGeometryShader += "in vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "[];\n";
		pointShadowPassGeometryShader += "out vec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS;\n";

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
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "[i];\n";
		pointShadowPassGeometryShader += "\t\t\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS = " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "[i];\n";
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
	bool requiresAlphaTest = initializationData && 
		(initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
		 initializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	
	if (requiresAlphaTest)
	{
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR) + "_GS\n";
		shadowPassFragmentShader += "#define " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + " " + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + "_GS\n";

		FragmentShaderInitializationData fragmentShaderInitializationData("", "");
		fragmentShaderInitializationData.materialInitializationData = initializationData;
		fragmentShaderInitializationData.shader = shader;
		fragmentShaderInitializationData.renderPassType = RenderPassType::PointLightShadow;
		
		shadowPassFragmentShader += General_FS_GetMaterialVariables(fragmentShaderInitializationData);
		shadowPassFragmentShader += General_FS_GetShaderTextureUniforms(initializationData, shader);

		if (!initializationData->fragmentShaderFunctions.empty())
		{
			shadowPassFragmentShader += initializationData->fragmentShaderFunctions;
		}

		if (!initializationData->fragmentShaderUniforms.empty())
		{
			shadowPassFragmentShader += initializationData->fragmentShaderUniforms;
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
		shadowPassFragmentShader += FS_InitializeBaseColor(initializationData, true);
		shadowPassFragmentShader += FS_InitializeEmmisiveColor(initializationData);
		shadowPassFragmentShader += "\n\tif (1.f < length(" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR) + ")) discard;\n";
	}

	shadowPassFragmentShader += R"(
	float lightDistance = length()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(.xyz - )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_POSITION + R"();
    lightDistance /= )" + SHADER_VARIABLE_NAMES::SHADOW::LIGHT_RADIUS + R"(;
	gl_FragDepth = lightDistance;
})";
	return shadowPassFragmentShader;
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

std::string ShaderBuilder::GeometryBufferPass_GetOutputVariables() const
{
	std::string variables = R"(

layout(location = 0) out vec3 )" + std::string(SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION) + R"(;
layout(location = 1) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
layout(location = 2) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
layout(location = 3) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS + R"(;
layout(location = 4) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR + R"(;

)";

	return variables;
}

std::string ShaderBuilder::GeometryBufferPass_GetOutputVariableAssignments() const
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
	assignments += " = ";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR;
	assignments += ".xyz;\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS;
	assignments += " = vec3(finalAmbientOcclusion, finalMetallic, finalRoughness);\n";

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

	assignments += "\tvec3 ambientOcclusionMetallicRoughness = texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_AMBIENT_OCCLUSION_METALLIC_ROUGHNESS;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ").xyz;\n";
	assignments += "\tfinalAmbientOcclusion = ambientOcclusionMetallicRoughness.x;\n";
	assignments += "\tfinalMetallic = ambientOcclusionMetallicRoughness.y;\n";
	assignments += "\tfinalRoughness = max(ambientOcclusionMetallicRoughness.z, 0.04f);\n\n";

	assignments += "\t";
	assignments += SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR;
	assignments += " = texture(";
	assignments += SHADER_VARIABLE_NAMES::GBUFFER::OUT_EMISIVE_COLOR;
	assignments += ", ";
	assignments += SHADER_VARIABLE_NAMES::TEXTURE::UV;
	assignments += ").xyz;\n";

	return assignments;
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
	materialVariableText += "vec3 surfaceNormal;\n";

	bool requiresAlphaTest = fragmentShaderInitializationData.materialInitializationData && 
		(fragmentShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked || 
		 fragmentShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);

	bool includeMaterialVariables = fragmentShaderInitializationData.renderPassType == RenderPassType::Forward ||
		fragmentShaderInitializationData.renderPassType == RenderPassType::GeometryBuffer ||
		(requiresAlphaTest && (fragmentShaderInitializationData.renderPassType == RenderPassType::Shadow || fragmentShaderInitializationData.renderPassType == RenderPassType::PointLightShadow));

	if(includeMaterialVariables)
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

		materialVariableText += "uniform vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR;
		materialVariableText += ";\n";

		materialVariableText += "in vec3 ";
		materialVariableText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL;
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
	float lightDistance = length(lightDirection);

	if(radius < lightDistance || lightDistance <= 0.f)
	{
		return vec3(0.f);
	}

	lightDirection /= lightDistance;
	vec3 radiance = intensity / (lightDistance * lightDistance);
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
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
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
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
	float )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::SHADOW_INTENSITY + R"(;
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
	bool )" + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + R"(;
};
)";
	return spotLightStruct;
}

std::string ShaderBuilder::FS_GetLightArrayUniforms() const
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

std::string ShaderBuilder::FS_GetShadowMapUniforms() const
{
	return R"(
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_DIRECTIONAL_LIGHT_COUNT) + "]" + R"(;
uniform samplerCubeShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_POINT_LIGHT_COUNT) + "]" + R"(;
uniform sampler2DShadow )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME) + "[" + std::to_string(MAX_SPOT_LIGHT_COUNT) + "]" + R"(;
)";
}

std::string ShaderBuilder::General_FS_GetShaderTextureUniforms(MaterialInitializationData* initializationData, const Shader* shader) const
{
	if (!shader)
	{
		return "";
	}

	std::string uniforms = "";

	const std::vector<const Texture*>* textures = shader->GetTextures();

	std::unordered_set<std::string> addedTextureNames;

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
		case TextureUsage::Emmisive:
			if (initializationData && initializationData->emisiveColor.result.empty())
			{
				initializationData->emisiveColor.result = General_FS_GetEmmisiveTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::AmbientOcclusion:
			if (initializationData && initializationData->ambientOcclusion.result.empty())
			{
				initializationData->ambientOcclusion.result = General_FS_GetScalarTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::Metallic:
			if (initializationData && initializationData->metallic.result.empty())
			{
				initializationData->metallic.result = General_FS_GetScalarTextureSampling(texture->GetName());
			}
			break;
		case TextureUsage::Roughness:
			if (initializationData && initializationData->roughness.result.empty())
			{
				initializationData->roughness.result = General_FS_GetScalarTextureSampling(texture->GetName());
			}
			break;
		default:
			break;
		}

		if (addedTextureNames.find(texture->GetName()) == addedTextureNames.end())
		{
			uniforms += "uniform sampler2D " + texture->GetName() + ";\n";
			addedTextureNames.insert(texture->GetName());
		}

		++textureIterator;
	}

	return uniforms;
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
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").xyz * 2.f - vec3(1.f); ");
}

std::string ShaderBuilder::General_FS_GetEmmisiveTextureSampling(const std::string& textureName) const
{
	return std::string("texture(" + textureName + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + ").xyz; ");
}

std::string ShaderBuilder::FS_GetLightCalculationIterators() const
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
			vec3 )" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"(.xyz + geometryNormal * 0.025f - )" + SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_ARRAY_NAME + "[pointLightIndex]." + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
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
		(initializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
		 (discardTransparent && initializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent)))
	{
		result += "\tif (" + std::string(SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR) + ".a < 0.5f) discard;\n";
	}

	return result;
}

std::string ShaderBuilder::FS_InitializeEmmisiveColor(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (initializationData && !initializationData->emisiveColor.calculation.empty())
	{
		result += initializationData->emisiveColor.calculation + "\n";
	}
	
	result += std::string("\n\t") + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_EMMISIVE_COLOR + " = ";

	if (initializationData && !initializationData->emisiveColor.result.empty())
	{
		result += initializationData->emisiveColor.result;
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

	result += "\tsurfaceNormal = geometryNormal;\n";

	if (initializationData && !initializationData->fragmentNormal.calculation.empty())
	{
		result += initializationData->fragmentNormal.calculation + "\n";
	}

	if (initializationData && !initializationData->fragmentNormal.result.empty())
	{
		result += "\n\tsurfaceNormal = ";
		result += initializationData->fragmentNormal.result;
	}

	result += "\tsurfaceNormal = normalize(surfaceNormal);\n";
	return result;
}

std::string ShaderBuilder::FS_GetPBRFunctions() const
{
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

vec3 CalculatePBRLighting(vec3 lightDirection, vec3 radiance)
{
    vec3 normal = surfaceNormal;
    vec3 viewDirection = normalize()" + std::string(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION) + R"( - vec3()" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"());
    
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

    vec3 F0 = mix(vec3(0.04f), )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb, finalMetallic);
    vec3 specular = vec3(0.f);
    vec3 kS = vec3(0.f);

    // Only calculate specular reflections for front-facing light
    if(!isBackface) 
    {
        vec3 halfVector = normalize(viewDirection + lightDirection);
        kS = FresnelSchlick(max(dot(halfVector, viewDirection), 0.f), F0);
        
        float NDF = DistributionGGX(normal, halfVector, finalRoughness);
        float G = GeometrySmith(normal, viewDirection, lightDirection, finalRoughness);

        vec3 numerator = NDF * G * kS;
        float denominator = max(4.f * normalDotView * clampedNormalDotLight, 0.0001f);
        specular = numerator / denominator;
    }

    vec3 kD = (vec3(1.f) - kS) * (1.f - finalMetallic);

    // FIX: Divided base color by PI for energy conservation
    vec3 directLighting = ((kD * )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb / PI) + specular) * radiance * clampedNormalDotLight;

    if(isBackface)
    {
        directLighting *= )" + SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY + R"(;
    }

    return directLighting;
}

vec3 CalculatePBRAmbientLight()
{
    // A simple ambient approximation. 
    return 0.12f * )" + SHADER_VARIABLE_NAMES::CALCULATIONS::FINAL_BASE_COLOR + R"(.rgb * finalAmbientOcclusion;
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

	return layouts;
}

std::string ShaderBuilder::VS_GetInstancedStaticMeshLayouts() const
{
	std::string layouts = "\n\n";

	layouts += "layout(location = 4) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_0;
	layouts += ";\n";

	layouts += "layout(location = 5) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_1;
	layouts += ";\n";

	layouts += "layout(location = 6) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_2;
	layouts += ";\n";

	layouts += "layout(location = 7) in vec4 ";
	layouts += SHADER_VARIABLE_NAMES::VERTEX::INSTANCE_TRANSFORMATION_ROW_3;
	layouts += ";\n";

	return layouts;
}

std::string ShaderBuilder::VS_GetSkeletalMeshLayouts() const
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
	vsMain += VS_GetPositionOffset(vertexShaderInitializationData.materialInitializationData);

	vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable + ";\n";
	vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"( = vec4()" + SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION + R"(, 1.f)* )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";\n";
	
	bool isPointLightShadowPass = vertexShaderInitializationData.renderPassType == RenderPassType::PointLightShadow;
	bool requiresAlphaTest = vertexShaderInitializationData.materialInitializationData && 
		(vertexShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Masked ||
		 vertexShaderInitializationData.materialInitializationData->owner->GetBlendModel() == MaterialBlendModel::Transparent);
	
	if (!isPointLightShadowPass || requiresAlphaTest)
	{
		vsMain += "\n\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + ";\n";
	}

	if (vertexShaderInitializationData.renderPassType == RenderPassType::Forward)
	{
		vsMain += VS_GetLightSpaceFragmentPositionCalculations();
	}

	if (!isPointLightShadowPass || requiresAlphaTest)
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

std::string ShaderBuilder::VS_GetPosition() const
{
	return "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " = " + std::string(SHADER_VARIABLE_NAMES::VERTEX::POSITION) + ";\n";
}

std::string ShaderBuilder::VS_GetPositionOffset(MaterialInitializationData* initializationData) const
{
	std::string result = "";

	if (!initializationData->vertexPositionOffset.result.empty())
	{
		result += initializationData->vertexPositionOffset.calculation + "\n";
		result += "\t" + std::string(SHADER_VARIABLE_NAMES::VERTEX::MODIFIED_POSITION) + " += " + initializationData->vertexPositionOffset.result + ";\n";
	}

	return result;
}

std::string ShaderBuilder::VS_GetUV(MaterialInitializationData* initializationData) const
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

std::string ShaderBuilder::VS_GetVertexColorText() const
{
	std::string vertexColorText = "";

	vertexColorText += "\n\t";
	vertexColorText += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_COLOR;
	vertexColorText += " = ";
	vertexColorText += SHADER_VARIABLE_NAMES::VERTEX::COLOR;
	vertexColorText += ";\n";

	return vertexColorText;
}
