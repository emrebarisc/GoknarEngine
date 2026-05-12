#include "pch.h"

#include "BillboardParticleSystem.h"

#include "Goknar/Camera.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialInstance.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

namespace
{
	const Material* ResolveMaterialTemplate(const IMaterialBase* material)
	{
		if (const Material* typedMaterial = dynamic_cast<const Material*>(material))
		{
			return typedMaterial;
		}

		if (const MaterialInstance* materialInstance = dynamic_cast<const MaterialInstance*>(material))
		{
			return materialInstance->GetParentMaterial();
		}

		return nullptr;
	}

	void AddMaterialTexturesToShader(const IMaterialBase* material, Shader* shader)
	{
		if (!material || !shader)
		{
			return;
		}

		const std::vector<const Image*>* textureImages = material->GetTextureImages();
		if (!textureImages)
		{
			return;
		}

		for (const Image* textureImage : *textureImages)
		{
			if (!textureImage || !textureImage->GetGeneratedTexture())
			{
				continue;
			}

			shader->AddTexture(textureImage->GetGeneratedTexture());
		}
	}
}

BillboardParticleSystem::BillboardParticleSystem(const GPUParticleSystemDesc& desc) :
	ParticleSystemBase(desc)
{
}

BillboardParticleSystem::~BillboardParticleSystem()
{
	DestroyRenderResources();
}

void BillboardParticleSystem::Render(const Camera* activeCamera) const
{
	if (!GetIsInitialized() || !activeCamera || GetDrawIndirectBufferId() == 0 || !renderShader_)
	{
		return;
	}

	BindRenderBuffers();

	renderShader_->Use();
	renderShader_->SetMVP(Matrix::IdentityMatrix);
	renderShader_->SetVector3(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_RIGHT, -activeCamera->GetLeftVector());
	renderShader_->SetVector3(SHADER_VARIABLE_NAMES::PARTICLE::CAMERA_UP, activeCamera->GetUpVector());
	ApplyParticleStateToShader(renderShader_);
	ApplyMaterialStateToShader(renderShader_, particleMaterial_);
	engine->SetShaderEngineVariables(renderShader_);

	engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, false);
	engine->GetGraphicsAPI()->BindVertexArray(GetDummyVertexArrayObjectId());
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, GetDrawIndirectBufferId());
	engine->GetGraphicsAPI()->DrawArraysIndirect(GraphicsPrimitive::Triangles, nullptr);
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, 0);
	engine->GetGraphicsAPI()->BindVertexArray(0);
	engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
}

void BillboardParticleSystem::SetParticleTexture(const Image* particleTexture)
{
	if (particleTexture_ == particleTexture)
	{
		return;
	}

	particleTexture_ = particleTexture;

	if (GetIsInitialized())
	{
		RecreateRenderShader();
	}
}

void BillboardParticleSystem::SetParticleMaterial(const IMaterialBase* particleMaterial)
{
	if (particleMaterial_ == particleMaterial)
	{
		return;
	}

	particleMaterial_ = particleMaterial;

	if (GetIsInitialized())
	{
		RecreateRenderShader();
	}
}

void BillboardParticleSystem::CreateRenderResources()
{
	renderShader_ = new Shader();

	if (particleMaterial_)
	{
		AddMaterialTexturesToShader(particleMaterial_, renderShader_);
	}
	else if (particleTexture_ && particleTexture_->GetGeneratedTexture())
	{
		renderShader_->AddTexture(particleTexture_->GetGeneratedTexture());
	}

	MaterialInitializationData particleInitializationData(particleMaterial_);
	MaterialInitializationData* particleInitializationDataPtr = nullptr;

	if (const Material* materialTemplate = ResolveMaterialTemplate(particleMaterial_))
	{
		particleInitializationData = *materialTemplate->GetInitializationData();
		particleInitializationData.owner = particleMaterial_;
		particleInitializationDataPtr = &particleInitializationData;
	}
	else
	{
		particleInitializationData.baseColor.calculation =
			"\tvec2 particleUvCentered = " + std::string(SHADER_VARIABLE_NAMES::TEXTURE::UV) + " * 2.0 - 1.0;\n"
			"\tfloat particleRadialDistance = dot(particleUvCentered, particleUvCentered);\n"
			"\tif (1.0 < particleRadialDistance) discard;\n"
			"\tfloat particleRadialFalloff = 1.0 - smoothstep(0.0, 1.0, particleRadialDistance);\n";
		particleInitializationData.baseColor.result = "vec4(vec3(particleRadialFalloff), particleRadialFalloff)";
		particleInitializationDataPtr = &particleInitializationData;

		if (particleTexture_)
		{
			particleInitializationData.baseColor.calculation.clear();
			particleInitializationData.baseColor.result.clear();
		}
	}

	renderShader_->SetVertexShaderScript(
		ShaderBuilder::GetInstance()->ParticleRenderPass_GetBillboardVertexShaderScript(particleInitializationDataPtr, renderShader_));
	renderShader_->SetFragmentShaderScript(
		ShaderBuilder::GetInstance()->ParticleRenderPass_GetFragmentShaderScript(particleInitializationDataPtr, renderShader_));
	renderShader_->PreInit();
	renderShader_->Init();

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->BindShadowTextures(renderShader_);
	}
}

void BillboardParticleSystem::DestroyRenderResources()
{
	delete renderShader_;
	renderShader_ = nullptr;
}

void BillboardParticleSystem::OnInit()
{
}

void BillboardParticleSystem::OnPostInit()
{
	if (renderShader_)
	{
		renderShader_->PostInit();
	}
}

void BillboardParticleSystem::RecreateDrawIndirectBuffer()
{
	if (GetDrawIndirectBufferId() == 0)
	{
		return;
	}

	const GPUParticleDrawArraysIndirectCommand drawCommand{};
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, GetDrawIndirectBufferId());
	engine->GetGraphicsAPI()->BufferData(GraphicsBufferTarget::ShaderStorageBuffer, sizeof(GPUParticleDrawArraysIndirectCommand), &drawCommand, GraphicsBufferUsage::DynamicDraw);
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0);
}

void BillboardParticleSystem::DispatchFinalizePass() const
{
	BindFinalizeBuffers();

	if (ComputeShader* finalizeComputeShader = GetFinalizeDrawArraysComputeShader())
	{
		finalizeComputeShader->Dispatch(1u, 1u, 1u);
	}
}

void BillboardParticleSystem::RecreateRenderShader()
{
	DestroyRenderResources();
	CreateRenderResources();
	OnPostInit();
}
