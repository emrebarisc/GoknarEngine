 #include "pch.h"

#include "Material.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Contents/Image.h"

Material::Material() : 
	ambientReflectance_(Vector3::ZeroVector), 
	diffuseReflectance_(Vector3::ZeroVector), 
	specularReflectance_(Vector3::ZeroVector),
	phongExponent_(1.f),
	blendModel_(MaterialBlendModel::Opaque),
	shadingModel_(MaterialShadingModel::Default)
{
}

Material::~Material()
{
	delete renderPassTypeShaderMap_[RenderPassType::Main];
	delete renderPassTypeShaderMap_[RenderPassType::GBuffer];
	delete renderPassTypeShaderMap_[RenderPassType::Shadow];
}

void Material::Build(MeshUnit* meshUnit)
{
	Shader* mainShader = new Shader();
	std::vector<const Image*>::iterator imageIterator = textureImages_.begin();
	for (; imageIterator != textureImages_.end(); ++imageIterator)
	{
		mainShader->AddTexture((*imageIterator)->GetGeneratedTexture());
	}

	renderPassTypeShaderMap_[RenderPassType::Main] = mainShader;
	if (mainShader->GetShaderType() == ShaderType::Scene)
	{
		ShaderBuilder::GetInstance()->BuildShader(meshUnit, this);
	}

	const std::string& mainShaderVertexShaderScript = mainShader->GetVertexShaderScript();

	Shader* shadowShader = new Shader();
	shadowShader->SetVertexShaderScript(mainShaderVertexShaderScript);
	shadowShader->SetFragmentShaderScript(ShaderBuilder::GetShaderPassFragmentShaderScript());
	renderPassTypeShaderMap_[RenderPassType::Shadow] = shadowShader;

}

void Material::PreInit()
{
	renderPassTypeShaderMap_[RenderPassType::Main]->PreInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PreInit();
}

void Material::Init()
{
	renderPassTypeShaderMap_[RenderPassType::Main]->Init();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->Init();
}

void Material::PostInit()
{
	renderPassTypeShaderMap_[RenderPassType::Main]->PostInit();
	renderPassTypeShaderMap_[RenderPassType::Shadow]->PostInit();
}

void Material::Render(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType) const
{
	Use(renderPassType);
	SetShaderVariables(worldTransformationMatrix, relativeTransformationMatrix, renderPassType);
}

void Material::Use(RenderPassType renderPassType) const
{
	GOKNAR_CORE_ASSERT(renderPassTypeShaderMap_.find(renderPassType) != renderPassTypeShaderMap_.end());
	
	renderPassTypeShaderMap_.at(renderPassType)->Use();
}

void Material::SetShaderVariables(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix, RenderPassType renderPassType) const
{
	GOKNAR_CORE_ASSERT(renderPassTypeShaderMap_.find(renderPassType) != renderPassTypeShaderMap_.end());

	Shader* shader = renderPassTypeShaderMap_.at(renderPassType);

	if (renderPassType == RenderPassType::Main)
	{
		if (shadingModel_ == MaterialShadingModel::Default)
		{
			glEnable(GL_CULL_FACE);
		}
		else if (shadingModel_ == MaterialShadingModel::TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, ambientReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, diffuseReflectance_);
		shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, specularReflectance_);
		shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, phongExponent_);
	}
	else if (renderPassType == RenderPassType::Shadow)
	{

	}

	shader->SetMVP(worldTransformationMatrix, relativeTransformationMatrix);
	engine->SetShaderEngineVariables(shader);
}
