#include "pch.h"

#include "StaticMeshParticleSystem.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialInstance.h"
#include "Goknar/Model/MeshGeometry.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/ComputeShader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderTypes.h"

#include <limits>

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

	bool ShouldRenderMaterial(const IMaterialBase* material, ParticleRenderStage renderStage)
	{
		if (renderStage == ParticleRenderStage::All)
		{
			return true;
		}

		const bool isTransparent = material && material->GetBlendModel() == MaterialBlendModel::Transparent;
		return renderStage == ParticleRenderStage::Transparent ? isTransparent : !isTransparent;
	}
}

StaticMeshParticleSystem::StaticMeshParticleSystem(const GPUParticleSystemDesc& desc) :
	ParticleSystemBase(desc)
{
}

StaticMeshParticleSystem::~StaticMeshParticleSystem()
{
	DestroyRenderResources();
}

std::uint32_t StaticMeshParticleSystem::Render(const Camera*, ParticleRenderStage renderStage) const
{
	if (!GetIsInitialized() || GetDrawIndirectBufferId() == 0 || staticMeshSubmeshRenderData_.empty() || !engine || !engine->GetRenderer())
	{
		return 0u;
	}

	BindRenderBuffers();

	engine->GetGraphicsAPI()->BindVertexArray(GetDummyVertexArrayObjectId());
	engine->GetRenderer()->BindStaticMeshBuffers();
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, GetDrawIndirectBufferId());
	std::uint32_t drawCount = 0u;

	for (std::uint32_t commandIndex = 0u; commandIndex < static_cast<std::uint32_t>(staticMeshSubmeshRenderData_.size()); ++commandIndex)
	{
		const StaticMeshSubmeshRenderData& renderData = staticMeshSubmeshRenderData_[commandIndex];
		if (!renderData.renderShader || !ShouldRenderMaterial(renderData.material, renderStage))
		{
			continue;
		}

		renderData.renderShader->Use();
		renderData.renderShader->SetMVP(Matrix::IdentityMatrix);
		ApplyParticleStateToShader(renderData.renderShader);
		ApplyMaterialStateToShader(renderData.renderShader, renderData.material);
		engine->SetShaderEngineVariables(renderData.renderShader);

		if (renderData.material && renderData.material->GetShadingModel() == MaterialShadingModel::TwoSided)
		{
			engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, false);
		}
		else
		{
			engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
		}

		const void* indirectCommandOffset = reinterpret_cast<const void*>(
			static_cast<std::uintptr_t>(commandIndex * sizeof(GPUParticleDrawElementsIndirectCommand)));
		engine->GetGraphicsAPI()->DrawElementsIndirect(GraphicsPrimitive::Triangles, GraphicsDataType::UnsignedInt, indirectCommandOffset);
		++drawCount;
	}

	engine->GetGraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, 0);
	engine->GetGraphicsAPI()->BindVertexArray(0);

	return drawCount;
}

void StaticMeshParticleSystem::SetStaticMesh(const StaticMeshLOD* staticMesh)
{
	if (staticMeshLOD_ == staticMesh)
	{
		return;
	}

	staticMeshLOD_ = staticMesh;

	if (!GetIsInitialized())
	{
		return;
	}

	RefreshStaticMeshSubmeshRenderData();
	OnPostInit();
	RecreateDrawIndirectBuffer();
	ResetSimulationState();
}

void StaticMeshParticleSystem::CreateRenderResources()
{
}

void StaticMeshParticleSystem::DestroyRenderResources()
{
	for (StaticMeshSubmeshRenderData& renderData : staticMeshSubmeshRenderData_)
	{
		delete renderData.renderShader;
		renderData.renderShader = nullptr;
	}
}

void StaticMeshParticleSystem::OnInit()
{
	RefreshStaticMeshSubmeshRenderData();
}

void StaticMeshParticleSystem::OnPostInit()
{
	for (StaticMeshSubmeshRenderData& renderData : staticMeshSubmeshRenderData_)
	{
		if (renderData.renderShader)
		{
			renderData.renderShader->PostInit();
		}
	}
}

void StaticMeshParticleSystem::RecreateDrawIndirectBuffer()
{
	if (GetDrawIndirectBufferId() == 0)
	{
		return;
	}

	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, GetDrawIndirectBufferId());

	if (!staticMeshSubmeshRenderData_.empty())
	{
		std::vector<GPUParticleDrawElementsIndirectCommand> drawCommands;
		drawCommands.reserve(static_cast<std::size_t>(staticMeshSubmeshRenderData_.size()));

		for (const StaticMeshSubmeshRenderData& renderData : staticMeshSubmeshRenderData_)
		{
			GPUParticleDrawElementsIndirectCommand drawCommand{};
			drawCommand.indexCountPerInstance = renderData.indexCount;
			drawCommand.instanceCount = 0u;
			drawCommand.firstIndex = renderData.firstIndex;
			drawCommand.baseVertex = renderData.baseVertex;
			drawCommand.baseInstance = 0u;
			drawCommands.push_back(drawCommand);
		}

		engine->GetGraphicsAPI()->BufferData(
			GraphicsBufferTarget::ShaderStorageBuffer,
			static_cast<GEsizeiptr>(drawCommands.size() * sizeof(GPUParticleDrawElementsIndirectCommand)),
			drawCommands.data(),
			GraphicsBufferUsage::DynamicDraw);
	}
	else
	{
		const GPUParticleDrawElementsIndirectCommand drawCommand{};
		engine->GetGraphicsAPI()->BufferData(GraphicsBufferTarget::ShaderStorageBuffer, sizeof(GPUParticleDrawElementsIndirectCommand), &drawCommand, GraphicsBufferUsage::DynamicDraw);
	}

	engine->GetGraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0);
}

void StaticMeshParticleSystem::DispatchFinalizePass() const
{
	if (staticMeshSubmeshRenderData_.empty())
	{
		return;
	}

	BindFinalizeBuffers();

	ComputeShader* finalizeComputeShader = GetFinalizeDrawElementsComputeShader();
	if (!finalizeComputeShader)
	{
		return;
	}

	const GEuint commandCount = static_cast<GEuint>(staticMeshSubmeshRenderData_.size());
	finalizeComputeShader->Use();
	finalizeComputeShader->SetInt("commandCount", static_cast<int>(commandCount));

	const GEuint groupCountX = (commandCount + FINALIZE_DRAW_LOCAL_SIZE_X - 1u) / FINALIZE_DRAW_LOCAL_SIZE_X;
	finalizeComputeShader->Dispatch(groupCountX, 1u, 1u);
}

Shader* StaticMeshParticleSystem::CreateRenderShaderForMaterial(const IMaterialBase* material) const
{
	Shader* renderShader = new Shader();
	AddMaterialTexturesToShader(material, renderShader);

	MaterialInitializationData particleInitializationData(material);
	MaterialInitializationData* particleInitializationDataPtr = nullptr;
	if (const Material* materialTemplate = ResolveMaterialTemplate(material))
	{
		particleInitializationData = *materialTemplate->GetInitializationData();
		particleInitializationData.owner = material;
		particleInitializationDataPtr = &particleInitializationData;
	}

	renderShader->SetVertexShaderScript(
		ShaderBuilder::GetInstance()->ParticleRenderPass_GetStaticMeshVertexShaderScript(particleInitializationDataPtr, renderShader));
	renderShader->SetFragmentShaderScript(
		ShaderBuilder::GetInstance()->ParticleRenderPass_GetFragmentShaderScript(particleInitializationDataPtr, renderShader));
	renderShader->PreInit();
	renderShader->Init();

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->BindShadowTextures(renderShader);
	}

	return renderShader;
}

void StaticMeshParticleSystem::RefreshStaticMeshSubmeshRenderData()
{
	DestroyRenderResources();
	staticMeshSubmeshRenderData_.clear();

	if (!staticMeshLOD_)
	{
		return;
	}

	const std::vector<MeshGeometry*>& subMeshes = staticMeshLOD_->GetSubMeshes();
	for (const MeshGeometry* subMesh : subMeshes)
	{
		if (!subMesh)
		{
			continue;
		}

		const std::uint32_t indexCount = subMesh->GetFaceCount() * 3u;
		if (indexCount == 0u)
		{
			continue;
		}

		const std::uint32_t indexElementSizeInBytes = sizeof(Face::vertexIndices[0]);
		GOKNAR_CORE_ASSERT(
			subMesh->GetVertexStartingIndex() % indexElementSizeInBytes == 0u,
			"Static mesh particle indirect draw expects index-buffer offsets aligned to index element size.");
		GOKNAR_CORE_ASSERT(
			subMesh->GetBaseVertex() <= static_cast<unsigned int>((std::numeric_limits<std::int32_t>::max)()),
			"Static mesh particle indirect draw base vertex exceeds OpenGL indirect command range.");

		StaticMeshSubmeshRenderData renderData;
		renderData.meshUnit = subMesh;
		renderData.material = subMesh->GetMaterialBase();
		renderData.renderShader = CreateRenderShaderForMaterial(renderData.material);
		renderData.indexCount = indexCount;
		renderData.firstIndex = subMesh->GetVertexStartingIndex() / indexElementSizeInBytes;
		renderData.baseVertex = static_cast<std::int32_t>(subMesh->GetBaseVertex());
		staticMeshSubmeshRenderData_.push_back(renderData);
	}

}
