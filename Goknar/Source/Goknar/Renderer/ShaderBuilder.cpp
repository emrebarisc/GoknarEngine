#include "pch.h"
#include "ShaderBuilder.h"

#include "Goknar/Application.h"
#include "Goknar/Core.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Scene.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Material.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Renderer/Texture.h"

ShaderBuilder* ShaderBuilder::instance_ = nullptr;

void ShaderBuilder::BuildShader(MeshUnit* mesh, Material* material)
{
	GOKNAR_CORE_ASSERT(mesh);
	GOKNAR_CORE_ASSERT(material);

	Shader* shader = material->GetShader(RenderPassType::Forward);
	if (shader)
	{
		std::string vertexShader = BuildVertexShader(mesh);
		shader->SetVertexShaderScript(vertexShader);

		// Fragment Shader

		std::string fragmentShader;
		const std::vector<const Texture*>* textures = shader->GetTextures();
		size_t textureSize = textures->size();

		std::string fragmentShaderVariables = GetShaderVersionText();
		fragmentShaderVariables += GetMaterialVariables();
		fragmentShaderVariables += FS_GetVariableTexts();
		fragmentShaderVariables += GetLightShadowUniforms();
		fragmentShaderVariables += uniforms_;
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
		fragmentShaderMain += fragmentShaderShadowCalculation_;
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

		shader->SetFragmentShaderScript(fragmentShader);

		ResetVariables();
	}
}

std::string ShaderBuilder::BuildVertexShader(MeshUnit* mesh)
{
	// Vertex Shader
	std::string vertexShader = "// Default Vertex Shader";
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
	vertexShader += VS_GetLightUniforms();
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
	ResetVariables();
	return vertexShader;
}

std::string ShaderBuilder::BuildVertexShader_ShadowPass(MeshUnit* mesh)
{
	// Vertex Shader
	std::string vertexShader = "// ShadowPass Vertex Shader";
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

	vertexShader += "uniform mat4 ";
	vertexShader += SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX;
	vertexShader += ";\n";

	vertexShader += "uniform mat4 ";
	vertexShader += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX;
	vertexShader += ";\n";

	vertexShader += R"(
void main()
{
)";
	if (skeletalMesh)
	{
		vertexShader += VS_GetSkeletalMeshWeightCalculation();
	}
	vertexShader += VS_GetMain_ShadowPass();
	vertexShader += R"(
}
)";

	ResetVariables();
	return vertexShader;
}


std::string ShaderBuilder::BuildVertexShader_PointLightShadowPass(MeshUnit* mesh)
{
	// Vertex Shader
	std::string vertexShader = "// PointLightShadowPass Vertex Shader";
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
	vertexShader += VS_GetMain_PointLightShadowPass();
	vertexShader += R"(
}
)";

	ResetVariables();
	return vertexShader;
}

std::string ShaderBuilder::GetFragmentShaderScript_ShadowPass()
{
	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	shadowPassFragmentShader += R"(
void main()
{
})";
	return shadowPassFragmentShader;
}

std::string ShaderBuilder::GetFragmentShaderScript_PointLightShadowPass()
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

std::string ShaderBuilder::GetFragmentShaderScript_GeometryBufferPass(const Material* const material)
{
	const Shader* const shader = material->GetShader(RenderPassType::Forward);

	const std::vector<const Texture*>* textures = shader->GetTextures();
	size_t textureSize = textures->size();

	std::string fragmentShaderUniforms;

	for (size_t textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		const Texture* texture = textures->at(textureIndex);
		fragmentShaderUniforms += "uniform sampler2D " + texture->GetName() + ";\n";
	}

	std::string fragmentShaderInVariables = R"(
in vec4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
in vec3 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + ";\n";

	std::string insideMain;

	insideMain += std::string("\t") + SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION + " = " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + ".xyz;\n";
	insideMain += std::string("\t") + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + " = " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + ";\n";

	bool hasADiffuseShader = false;
	for (size_t textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		const Texture* texture = textures->at(textureIndex);
		if (texture->GetTextureUsage() == TextureUsage::Diffuse)
		{
			hasADiffuseShader = true;

			std::string textureColorVariable = texture->GetName() + "Color";

			insideMain += std::string("\tvec4 ") + textureColorVariable + " = texture(" + texture->GetName() + ", " + SHADER_VARIABLE_NAMES::TEXTURE::UV + "); \n";

			if (material->GetBlendModel() == MaterialBlendModel::Masked)
			{
				insideMain += "\tif (" + textureColorVariable + ".a < 0.5f) discard;\n";
			}

			insideMain += std::string("\t") + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + " = vec3(" + textureColorVariable + "); \n";
		}
	}

	if (hasADiffuseShader)
	{
		fragmentShaderInVariables += std::string("in vec2 ") + SHADER_VARIABLE_NAMES::TEXTURE::UV + "; ";
	}
	else
	{
		fragmentShaderUniforms += "uniform vec3 " + std::string(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE) + ";\n";
		insideMain += std::string("\t") + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + " = " + SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE + "; \n";
	}

	fragmentShaderUniforms += "uniform vec3 " + std::string(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR) + ";\n";
	insideMain += std::string("\t") + SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR + " = " + SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR + "; \n";

	std::string GBufferPassFragmentShader =
"#version " + std::string(DEFAULT_SHADER_VERSION) + R"(

layout(location = 0) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_POSITION + R"(;
layout(location = 1) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_NORMAL + R"(;
layout(location = 2) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_DIFFUSE + R"(;
layout(location = 3) out vec3 )" + SHADER_VARIABLE_NAMES::GBUFFER::OUT_SPECULAR + R"(;
)";

	GBufferPassFragmentShader += fragmentShaderUniforms;
	GBufferPassFragmentShader += fragmentShaderInVariables;

	GBufferPassFragmentShader += R"(
void main()
{
)";
	GBufferPassFragmentShader += insideMain;
	GBufferPassFragmentShader += R"(
})";
	return GBufferPassFragmentShader;
}

std::string ShaderBuilder::GetGeometryShaderScript_PointLightShadowPass()
{
	std::string shadowPassFragmentShader = "#version " + std::string(DEFAULT_SHADER_VERSION) + "\n";
	shadowPassFragmentShader += R"(
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
	return shadowPassFragmentShader;
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
	vertexShaderModelMatrixVariable_ = std::string(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX);
}

void ShaderBuilder::Init()
{
	ResetVariables();

	sceneVertexShader_ = VS_BuildScene();
	FS_BuildScene();
	isInstantiated_ = true;
}

std::string ShaderBuilder::GetShaderVersionText() const
{
	return "#version " + shaderVersion_ + "\n";
}

void ShaderBuilder::FS_BuildScene()
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	uniforms_ += FS_GetVariableTexts();
	uniforms_ += GetMaterialDiffuseVariable();

	const Vector3& sceneAmbientLight = scene->GetAmbientLight();
	fragmentShaderOutsideMain_ += "vec3 sceneAmbient = vec3(" + std::to_string(sceneAmbientLight.x / 255.f) + ", " + std::to_string(sceneAmbientLight.y / 255.f) + ", " + std::to_string(sceneAmbientLight.z / 255.f) + ");\n";
	fragmentShaderInsideMain_ += "\tvec3 " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + " = sceneAmbient * ambientReflectance;\n";

	bool writeShadowCalculationFunction = false;

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
				fragmentShaderOutsideMain_ += GetStaticPointLightText(staticPointLight);
				if (staticPointLight->GetIsShadowEnabled())
				{
					fragmentShaderInsideMain_ += PointLight_GetShadowCheck(lightVariableName);
					pointLightNamesForShaderSampler_.push_back(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetPointLightColorSummationText(lightVariableName);
				}
			}

			for (const PointLight* dynamicPointLight : dynamicPointLights)
			{
				std::string lightVariableName = dynamicPointLight->GetName();
				lightUniforms_ += GetPointLightUniformTexts(lightVariableName);
				if (dynamicPointLight->GetIsShadowEnabled())
				{
					fragmentShaderInsideMain_ += PointLight_GetShadowCheck(lightVariableName);
					pointLightNamesForShaderSampler_.push_back(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetPointLightColorSummationText(lightVariableName);
				}
			}
		}

		const std::vector<DirectionalLight*>& staticDirectionalLights = scene->GetStaticDirectionalLights();
		const std::vector<DirectionalLight*>& dynamicDirectionalLights = scene->GetDynamicDirectionalLights();
		if (staticDirectionalLights.size() > 0 || dynamicDirectionalLights.size() > 0)
		{
			for (const DirectionalLight* staticDirectionalLight : staticDirectionalLights)
			{
				std::string lightVariableName = staticDirectionalLight->GetName();
				fragmentShaderOutsideMain_ += GetStaticDirectionalLightText(staticDirectionalLight);

				if (staticDirectionalLight->GetIsShadowEnabled())
				{
					writeShadowCalculationFunction = true;
					directionalAndSpotLightNamesForShadowCalculation_.push_back(lightVariableName);
					fragmentShaderInsideMain_ += DirectionalLight_GetShadowCheck(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetDirectionalLightColorSummationText(lightVariableName);
				}
			}

			for (const DirectionalLight* dynamicDirectionalLight : dynamicDirectionalLights)
			{
				std::string lightVariableName = dynamicDirectionalLight->GetName();

				lightUniforms_ += GetDirectionalLightUniformTexts(lightVariableName);
				if (dynamicDirectionalLight->GetIsShadowEnabled())
				{
					writeShadowCalculationFunction = true;
					directionalAndSpotLightNamesForShadowCalculation_.push_back(lightVariableName);
					fragmentShaderInsideMain_ += DirectionalLight_GetShadowCheck(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetDirectionalLightColorSummationText(lightVariableName);
				}
			}

			fragmentShaderOutsideMain_ += GetDirectionalLightColorFunctionText() + "\n";
		}

		const std::vector<SpotLight*>& staticSpotLights = scene->GetStaticSpotLights();
		const std::vector<SpotLight*>& dynamicSpotLights = scene->GetDynamicSpotLights();
		if (staticSpotLights.size() > 0 || dynamicSpotLights.size() > 0)
		{
			fragmentShaderOutsideMain_ += GetSpotLightColorFunctionText() + "\n";

			for (const SpotLight* staticSpotLight : staticSpotLights)
			{
				std::string lightVariableName = staticSpotLight->GetName();
				fragmentShaderOutsideMain_ += GetStaticSpotLightText(staticSpotLight);

				if (staticSpotLight->GetIsShadowEnabled())
				{
					directionalAndSpotLightNamesForShadowCalculation_.push_back(lightVariableName);
					fragmentShaderInsideMain_ += SpotLight_GetShadowCheck(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetSpotLightColorSummationText(lightVariableName);
				}
			}

			for (const SpotLight* dynamicSpotLight : dynamicSpotLights)
			{
				std::string lightVariableName = dynamicSpotLight->GetName();
				lightUniforms_ += GetSpotLightUniformTexts(lightVariableName);

				if (dynamicSpotLight->GetIsShadowEnabled())
				{
					directionalAndSpotLightNamesForShadowCalculation_.push_back(lightVariableName);
					fragmentShaderInsideMain_ += SpotLight_GetShadowCheck(lightVariableName);
				}
				else
				{
					fragmentShaderInsideMain_ += GetSpotLightColorSummationText(lightVariableName);
				}
			}
		}
	}

	if (writeShadowCalculationFunction)
	{
		fragmentShaderOutsideMain_ += GetShadowCalculationFunction() + "\n";
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
	vertexNormalText += " = normalize(" + std::string(SHADER_VARIABLE_NAMES::VERTEX::NORMAL) + " * transpose(inverse(mat3(" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX + "))));\n";

	return vertexNormalText;
}

std::string ShaderBuilder::FS_GetVariableTexts()
{
	std::string variableTexts = "\n";
	variableTexts += "out vec3 ";
	variableTexts += SHADER_VARIABLE_NAMES::FRAGMENT_SHADER_OUTS::FRAGMENT_COLOR;
	variableTexts += ";\n";

	variableTexts += "in mat4 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX;
	variableTexts += ";\n";

	variableTexts += "in vec4 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE;
	variableTexts += ";\n";

	variableTexts += "in vec4 ";
	variableTexts += SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE;
	variableTexts += ";\n";

	for (const std::string& lightSpaceFragmentPositionLightName : directionalAndSpotLightNamesForShadowCalculation_)
	{
		variableTexts += std::string("in vec4 ") + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightSpaceFragmentPositionLightName + ";\n";
	}

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
	)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable_ + R"(;
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"( = vec4(position, 1.f) * )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";" + R"(
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX+ R"(;
	)";

	for (const std::string& lightSpaceFragmentPositionLightName : directionalAndSpotLightNamesForShadowCalculation_)
	{
		vsMain += std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightSpaceFragmentPositionLightName) + " = " 
			+ SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * "
			+ SHADER_VARIABLE_NAMES::SHADOW::VIEW_MATRIX_PREFIX + lightSpaceFragmentPositionLightName + "; \n";
	}
		
	vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
)";

	vsMain += std::string("\t") + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " = vec2(" + SHADER_VARIABLE_NAMES::VERTEX::UV + ".x, 1.f - " + SHADER_VARIABLE_NAMES::VERTEX::UV + ".y); \n";

	return vsMain;
}

std::string ShaderBuilder::VS_GetMain_ShadowPass()
{
	std::string vsMain = R"(
	mat4 )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable_ + R"(;
	vec4 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"( = vec4(position, 1.f) * )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";" + R"(
	vec4 )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + " * " + SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX + R"(;
	)";

	vsMain += R"(
	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_SCREEN_SPACE) + R"(;
)";

	return vsMain;
}

std::string ShaderBuilder::VS_GetMain_PointLightShadowPass()
{
	std::string vsMain = R"(
	)" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + " = " + vertexShaderModelMatrixVariable_ + R"(;
	)" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + R"( = vec4(position, 1.f) * )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + ";" + R"(

	gl_Position = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(;
)";

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

std::string ShaderBuilder::VS_GetUniforms()
{
	std::string uniforms = "";

	uniforms += "uniform mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX;
	uniforms += ";\n";

	uniforms += "uniform mat4 ";
	uniforms += SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX;
	uniforms += ";\n";

	for (const std::string& lightName : directionalAndSpotLightNamesForShadowCalculation_)
	{
		uniforms += std::string("uniform mat4 ") + SHADER_VARIABLE_NAMES::SHADOW::VIEW_MATRIX_PREFIX + lightName + ";\n";
	}

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

std::string ShaderBuilder::VS_GetLightUniforms()
{
	std::string uniforms = "";
	for (const std::string& lightName : directionalAndSpotLightNamesForShadowCalculation_)
	{
		uniforms += std::string("out vec4 ") + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightName + ";\n";
	}

	return uniforms;
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
		   "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ";\n" + 
		   "uniform float " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + ";\n";
}

std::string ShaderBuilder::GetPointLightColorFunctionText()
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

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"();
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

	color *= cosThetaPrime * intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderBuilder::GetStaticPointLightText(const PointLight* pointLight) const
{
	const Vector3& lightPosition = pointLight->GetPosition();
	const Vector3& lightColor = pointLight->GetColor();
	float lightIntensity = pointLight->GetIntensity();
	float radius = pointLight->GetRadius();
	bool isShadowEnabled = pointLight->GetIsShadowEnabled();

	std::string lightName = pointLight->GetName();

	return "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + " = vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ");\n" +
		   "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n" +
		   "float " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + " = " + std::to_string(radius) + ";\n";/* +
		   "bool " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::IS_CASTING_SHADOW + " = " + (isShadowEnabled ? "true" : "false") + ";\n";*/
}

std::string ShaderBuilder::GetPointLightColorSummationText(const std::string& lightVariableName)
{
	return "\t" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + " += CalculatePointLightColor(" +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + ");\n";
}

std::string ShaderBuilder::GetDirectionalLightUniformTexts(const std::string& lightVariableName)
{
	return "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ";\n" +
		   "uniform vec3 " + lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ";\n\n";
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
	vec3 wo = viewPosition - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"();
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= max(0, normalDotLightDirection) * intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}
)";
}

std::string ShaderBuilder::GetStaticDirectionalLightText(const DirectionalLight* directionalLight) const
{
	const Vector3& lightDirection = directionalLight->GetDirection();
	const Vector3& lightColor = directionalLight->GetColor();
	float lightIntensity = directionalLight->GetIntensity();

	std::string lightName = directionalLight->GetName();

	return  "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + " = vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ");\n" +
			"vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n";
}

std::string ShaderBuilder::GetDirectionalLightColorSummationText(const std::string& lightVariableName)
{
	return "\t" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + " += CalculateDirectionalLightColor(" +
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
	vec3 wi = vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"() - position;
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
	vec3 wo = normalize(viewPosition - vec3()" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"());

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

std::string ShaderBuilder::GetStaticSpotLightText(const SpotLight* spotLight) const
{
	const Vector3& lightPosition = spotLight->GetPosition();
	const Vector3& lightDirection = spotLight->GetDirection();
	const Vector3& lightColor = spotLight->GetColor();
	float lightIntensity = spotLight->GetIntensity();
	float coverageAngle = spotLight->GetCoverageAngle();
	float falloffAngle = spotLight->GetFalloffAngle();

	std::string lightName = spotLight->GetName();

	return  "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + " = vec3(" + std::to_string(lightPosition.x) + ", " + std::to_string(lightPosition.y) + ", " + std::to_string(lightPosition.z) + ");\n"
		  + "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + " = vec3(" + std::to_string(lightDirection.x) + ", " + std::to_string(lightDirection.y) + ", " + std::to_string(lightDirection.z) + ");\n"
		  + "vec3 " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + " = vec3(" + std::to_string(lightColor.x * lightIntensity) + ", " + std::to_string(lightColor.y * lightIntensity) + ", " + std::to_string(lightColor.z * lightIntensity) + ");\n"
		  + "float " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + " = " + std::to_string(coverageAngle) + ";\n"
		  + "float " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + " = " + std::to_string(falloffAngle) + ";\n";
}

std::string ShaderBuilder::GetSpotLightColorSummationText(const std::string& lightVariableName)
{
	return "\t" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + " += CalculateSpotLightColor(" +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ", " +
			lightVariableName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + ");\n";
}

std::string ShaderBuilder::GetLightShadowUniforms()
{
	std::string uniforms = "\n//---------------------------------------- SHADOW MAPS ----------------------------------------\n";
	for (const std::string& lightName : directionalAndSpotLightNamesForShadowCalculation_)
	{
		uniforms += std::string("uniform sampler2DShadow ") + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + lightName + ";\n";
	}

	for (const std::string& lightName : pointLightNamesForShaderSampler_)
	{
		uniforms += std::string("uniform samplerCubeShadow ") + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + lightName + ";\n";
	}
	uniforms += "//----------------------------------------------------------------------------------------------\n";
	uniforms += "\n";

	return uniforms;
}

std::string ShaderBuilder::PointLight_GetShadowCheck(const std::string& lightName)
{
	return R"(
	vec3 )" + std::string(SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR_PREFIX) + lightName + R"( = )" + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE + ".xyz + " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL + " * 0.025f - " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + R"(;
	float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"( = texture()" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + lightName + ", vec4(" + SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR_PREFIX + lightName + R"(, 0.15f)).x;
	)" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"( *= )" + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"(;
	if(length()" + std::string(SHADER_VARIABLE_NAMES::SHADOW::FRAGMENT_TO_LIGHT_VECTOR_PREFIX) + lightName + ") < " + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"()
	{
		vec3 )" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX + lightName + " = CalculatePointLightColor(" + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::RADIUS + R"();
		)" + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + " * " + SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX + lightName + R"(;
	}
)";
}

std::string ShaderBuilder::DirectionalLight_GetShadowCheck(const std::string& lightName)
{
	return R"(
	vec3 lightSpaceScreenCoordinate_)" + lightName + R"( = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX) + lightName + ".xyz / " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightName + R"(.w;

	if(	0.f <= lightSpaceScreenCoordinate_)" + lightName + R"(.x && lightSpaceScreenCoordinate_)" + lightName + R"(.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_)" + lightName + R"(.y && lightSpaceScreenCoordinate_)" + lightName + R"(.y <= 1.f)
	{
		float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"( = textureProj()" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + lightName + ", " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightName + R"();

		if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"()
		{
			vec3 )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX) + lightName + " = CalculateDirectionalLightColor(" + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"(); 
			)" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + " * " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX) + lightName + R"(;
		}
	}
	else
	{
		)" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + R"( += )" + " CalculateDirectionalLightColor(" + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " + lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + R"();
	}
)";
}

std::string ShaderBuilder::SpotLight_GetShadowCheck(const std::string& lightName)
{
	return R"(
	vec3 lightSpaceScreenCoordinate_)" + lightName + R"( = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX) + lightName + ".xyz / " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightName + R"(.w;

	if(	0.f <= lightSpaceScreenCoordinate_)" + lightName + R"(.x && lightSpaceScreenCoordinate_)" + lightName + R"(.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_)" + lightName + R"(.y && lightSpaceScreenCoordinate_)" + lightName + R"(.y <= 1.f)
	{
		float )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"( = textureProj()" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + lightName + ", " + SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_LIGHT_SPACE_PREFIX + lightName + R"();
		
		if(0.f < )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + R"()
		{
			vec3 )" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX) + lightName + " = CalculateSpotLightColor(" +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"(); 
			)" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + R"( += )" + SHADER_VARIABLE_NAMES::SHADOW::SHADOW_VALUE_PREFIX + lightName + " * " + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR_PREFIX) + lightName + R"(;
		}
	}
	else
	{
		)" + std::string(SHADER_VARIABLE_NAMES::LIGHT::LIGHT_COLOR) + R"( += )" + " CalculateSpotLightColor(" +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::POSITION + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::DIRECTION + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::INTENSITY + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::COVERAGE_ANGLE + ", " +
				lightName + SHADER_VARIABLE_NAMES::LIGHT_KEYWORDS::FALLOFF_ANGLE + R"();
	}
)";
}

std::string ShaderBuilder::GetShadowCalculationFunction()
{
	return R"()";
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
