#include "pch.h"

#include "GPUFoliageSystem.h"

#include "Goknar/Camera.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Lights/LightManager/LightManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Materials/MaterialInstance.h"
#include "Goknar/Model/MeshGeometry.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Renderer/ShaderBindingPoints.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"

#include <algorithm>
#include <limits>
#include <string>

namespace
{
	bool IsValidAABB(const Box& aabb)
	{
		const Vector3& min = aabb.GetMin();
		const Vector3& max = aabb.GetMax();
		return min.x <= max.x &&
			min.y <= max.y &&
			min.z <= max.z;
	}

	void ExtendBoundsWithPoint(Box& bounds, bool& hasBounds, const Vector3& point)
	{
		if (!hasBounds)
		{
			bounds = Box(point, point);
			hasBounds = true;
			return;
		}

		bounds.ExtendWRTPoint(point, false);
	}

	void AddTransformedAABBToBounds(
		const Box& localAABB,
		const Matrix& transformationMatrix,
		Box& bounds,
		bool& hasBounds)
	{
		if (!IsValidAABB(localAABB))
		{
			return;
		}

		const Vector3& min = localAABB.GetMin();
		const Vector3& max = localAABB.GetMax();
		for (int xIndex = 0; xIndex < 2; ++xIndex)
		{
			for (int yIndex = 0; yIndex < 2; ++yIndex)
			{
				for (int zIndex = 0; zIndex < 2; ++zIndex)
				{
					const Vector3 corner(
						xIndex == 0 ? min.x : max.x,
						yIndex == 0 ? min.y : max.y,
						zIndex == 0 ? min.z : max.z);
					ExtendBoundsWithPoint(bounds, hasBounds, Vector3(transformationMatrix * Vector4(corner, 1.f)));
				}
			}
		}
	}

	IGraphicsAPI* GraphicsAPI()
	{
		return engine->GetGraphicsAPI();
	}

	std::uint32_t ToUInt32Count(std::size_t count)
	{
		return static_cast<std::uint32_t>((std::min)(
			count,
			static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)())));
	}

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

	bool TryBuildGPUFoliageMaterialInitializationData(const IMaterialBase* material, MaterialInitializationData& outInitializationData)
	{
		const Material* materialTemplate = ResolveMaterialTemplate(material);
		if (!materialTemplate || !materialTemplate->GetInitializationData())
		{
			return false;
		}

		outInitializationData = *materialTemplate->GetInitializationData();
		outInitializationData.owner = material;
		outInitializationData.boneCount = 0;
		outInitializationData.meshType = MeshType::Static;
		return true;
	}

	void AddTextureIfMissing(const Texture* texture, Shader* shader, std::vector<const Texture*>& addedTextures)
	{
		if (!texture || !shader || std::find(addedTextures.begin(), addedTextures.end(), texture) != addedTextures.end())
		{
			return;
		}

		shader->AddTexture(texture);
		addedTextures.push_back(texture);
	}

	void AddTexturesFromShader(const Shader* sourceShader, Shader* targetShader, std::vector<const Texture*>& addedTextures)
	{
		if (!sourceShader || !targetShader || !sourceShader->GetTextures())
		{
			return;
		}

		for (const Texture* texture : *sourceShader->GetTextures())
		{
			AddTextureIfMissing(texture, targetShader, addedTextures);
		}
	}

	void AddMaterialTexturesToShader(const IMaterialBase* material, Shader* shader, RenderPassType preferredRenderPassType)
	{
		if (!material || !shader)
		{
			return;
		}

		std::vector<const Texture*> addedTextures;
		AddTexturesFromShader(material->GetShader(preferredRenderPassType), shader, addedTextures);
		AddTexturesFromShader(material->GetShader(RenderPassType::Forward), shader, addedTextures);
		AddTexturesFromShader(material->GetShader(RenderPassType::GeometryBuffer), shader, addedTextures);
		AddTexturesFromShader(material->GetShader(RenderPassType::Shadow), shader, addedTextures);
		AddTexturesFromShader(material->GetShader(RenderPassType::PointLightShadow), shader, addedTextures);

		if (!addedTextures.empty())
		{
			return;
		}

		const std::vector<const Image*>* textureImages = material->GetTextureImages();
		if (!textureImages)
		{
			return;
		}

		for (const Image* image : *textureImages)
		{
			AddTextureIfMissing(image ? image->GetGeneratedTexture() : nullptr, shader, addedTextures);
		}
	}
}

GPUFoliageSystem::GPUFoliageSystem()
{
	RegisterToRenderer();
}

GPUFoliageSystem::~GPUFoliageSystem()
{
	UnregisterFromRenderer();
	DestroyRenderBatches();
	DestroyBuffers();
}

void GPUFoliageSystem::PreInit()
{
	if (isInitialized_)
	{
		return;
	}

	CreateBuffers();
	isInitialized_ = true;
	RefreshRenderBatches();
	RefreshDrawBuffers();
}

void GPUFoliageSystem::PostInit()
{
	if (hasPostInitialized_)
	{
		return;
	}

	for (RenderBatch& renderBatch : renderBatches_)
	{
		if (renderBatch.forwardShader) renderBatch.forwardShader->PostInit();
		if (renderBatch.geometryBufferShader) renderBatch.geometryBufferShader->PostInit();
		if (renderBatch.shadowShader) renderBatch.shadowShader->PostInit();
		if (renderBatch.pointShadowShader) renderBatch.pointShadowShader->PostInit();
	}

	hasPostInitialized_ = true;
}

void GPUFoliageSystem::SetStaticMesh(const StaticMesh* staticMesh)
{
	if (staticMesh_ == staticMesh)
	{
		return;
	}

	staticMesh_ = staticMesh;
	staticMeshLOD_ = staticMesh_ ? staticMesh_->GetLOD(0) : nullptr;
	if (isInitialized_)
	{
		RefreshRenderBatches();
		RefreshDrawBuffers();
	}
	RecalculateAABB();
	instancesDirty_ = true;
	drawBuffersDirty_ = true;
}

void GPUFoliageSystem::SetInstances(const std::vector<GPUFoliageInstance>& instances, bool recalculateBounds)
{
	instances_ = instances;
	instancesDirty_ = true;
	drawBuffersDirty_ = true;

	if (recalculateBounds)
	{
		RecalculateAABB();
	}
}

void GPUFoliageSystem::SetInstanceTransformations(const std::vector<Matrix>& instanceTransformations, bool recalculateBounds)
{
	instances_.clear();
	instances_.reserve(instanceTransformations.size());
	for (const Matrix& instanceTransformation : instanceTransformations)
	{
		instances_.push_back(GPUFoliageInstance{ instanceTransformation, Vector4(1.f) });
	}

	instancesDirty_ = true;
	drawBuffersDirty_ = true;

	if (recalculateBounds)
	{
		RecalculateAABB();
	}
}

void GPUFoliageSystem::AddInstance(const GPUFoliageInstance& instance, bool recalculateBounds)
{
	instances_.push_back(instance);
	instancesDirty_ = true;
	drawBuffersDirty_ = true;

	if (recalculateBounds)
	{
		RecalculateAABB();
	}
}

void GPUFoliageSystem::AddInstanceTransformation(const Matrix& instanceTransformation, bool recalculateBounds)
{
	AddInstance(GPUFoliageInstance{ instanceTransformation, Vector4(1.f) }, recalculateBounds);
}

void GPUFoliageSystem::ClearInstances()
{
	instances_.clear();
	localAABB_ = Box();
	instancesDirty_ = true;
	drawBuffersDirty_ = true;
}

int GPUFoliageSystem::Render(const Camera* activeCamera, RenderPassType renderPassType, bool skipFrustumCulling)
{
	if (!isInitialized_ || !isActive_ || !staticMesh_ || instances_.empty())
	{
		return 0;
	}

	if ((renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow) && !castsShadow_)
	{
		return 0;
	}

	if (renderPassType == RenderPassType::Deferred || renderPassType == RenderPassType::CubemapCapture)
	{
		return 0;
	}

	if (!skipFrustumCulling && activeCamera && !activeCamera->IsAABBVisible(localAABB_, worldTransform_))
	{
		return 0;
	}

	UpdateStaticMeshLOD(activeCamera);
	if (!staticMeshLOD_)
	{
		return 0;
	}

	UploadInstanceBuffers();

	int drawCount = 0;
	RenderMeshBatches(renderPassType, drawCount);
	return drawCount;
}

void GPUFoliageSystem::RegisterToRenderer()
{
	if (!isRegistered_ && engine && engine->GetRenderer())
	{
		engine->GetRenderer()->AddGPUFoliageSystem(this);
		isRegistered_ = true;
	}
}

void GPUFoliageSystem::UnregisterFromRenderer()
{
	if (isRegistered_ && engine && engine->GetRenderer())
	{
		engine->GetRenderer()->RemoveGPUFoliageSystem(this);
		isRegistered_ = false;
	}
}

void GPUFoliageSystem::CreateBuffers()
{
	if (instanceBufferId_ == 0u)
	{
		instanceBufferId_ = GraphicsAPI()->CreateBuffer();
	}
	if (drawIndirectBufferId_ == 0u)
	{
		drawIndirectBufferId_ = GraphicsAPI()->CreateBuffer();
	}
}

void GPUFoliageSystem::DestroyBuffers()
{
	if (instanceBufferId_ != 0u)
	{
		GraphicsAPI()->DeleteBuffer(instanceBufferId_);
		instanceBufferId_ = 0u;
	}
	if (drawIndirectBufferId_ != 0u)
	{
		GraphicsAPI()->DeleteBuffer(drawIndirectBufferId_);
		drawIndirectBufferId_ = 0u;
	}
}

void GPUFoliageSystem::RefreshRenderBatches()
{
	DestroyRenderBatches();

	if (!isInitialized_ || !staticMeshLOD_)
	{
		return;
	}

	const std::vector<MeshGeometry*>& subMeshes = staticMeshLOD_->GetSubMeshes();
	for (const MeshGeometry* subMesh : subMeshes)
	{
		if (!subMesh || subMesh->GetFaceCount() == 0u)
		{
			continue;
		}

		RenderBatch renderBatch;
		renderBatch.meshUnit = subMesh;
		renderBatch.material = subMesh->GetMaterialBase();

		const std::uint32_t indexElementSizeInBytes = sizeof(Face::vertexIndices[0]);
		renderBatch.drawCommand.count = subMesh->GetFaceCount() * 3u;
		renderBatch.drawCommand.instanceCount = 0u;
		renderBatch.drawCommand.firstIndex = subMesh->GetVertexStartingIndex() / indexElementSizeInBytes;
		renderBatch.drawCommand.baseVertex = static_cast<std::int32_t>((std::min)(
			subMesh->GetBaseVertex(),
			static_cast<unsigned int>((std::numeric_limits<std::int32_t>::max)())));
		renderBatch.drawCommand.baseInstance = 0u;

		renderBatch.forwardShader = CreateMeshShader(renderBatch, RenderPassType::Forward);
		renderBatch.geometryBufferShader = CreateMeshShader(renderBatch, RenderPassType::GeometryBuffer);
		renderBatch.shadowShader = CreateMeshShader(renderBatch, RenderPassType::Shadow);
		renderBatch.pointShadowShader = CreateMeshShader(renderBatch, RenderPassType::PointLightShadow);

		renderBatches_.push_back(renderBatch);
	}

	drawBuffersDirty_ = true;
}

void GPUFoliageSystem::DestroyRenderBatches()
{
	for (RenderBatch& renderBatch : renderBatches_)
	{
		delete renderBatch.forwardShader;
		delete renderBatch.geometryBufferShader;
		delete renderBatch.shadowShader;
		delete renderBatch.pointShadowShader;
		renderBatch.forwardShader = nullptr;
		renderBatch.geometryBufferShader = nullptr;
		renderBatch.shadowShader = nullptr;
		renderBatch.pointShadowShader = nullptr;
	}
	renderBatches_.clear();
}

void GPUFoliageSystem::RefreshDrawBuffers()
{
	if (!isInitialized_)
	{
		return;
	}

	std::vector<DrawElementsIndirectCommand> drawCommands;
	drawCommands.reserve(renderBatches_.size());
	for (const RenderBatch& renderBatch : renderBatches_)
	{
		DrawElementsIndirectCommand drawCommand = renderBatch.drawCommand;
		drawCommand.instanceCount = ToUInt32Count(instances_.size());
		drawCommands.push_back(drawCommand);
	}

	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, drawIndirectBufferId_);
	GraphicsAPI()->BufferData(
		GraphicsBufferTarget::DrawIndirectBuffer,
		static_cast<GEsizeiptr>(drawCommands.size() * sizeof(DrawElementsIndirectCommand)),
		drawCommands.empty() ? nullptr : drawCommands.data(),
		GraphicsBufferUsage::DynamicDraw);
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, 0u);

	drawBuffersDirty_ = false;
}

void GPUFoliageSystem::UploadInstanceBuffers()
{
	if (instancesDirty_)
	{
		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, instanceBufferId_);
		GraphicsAPI()->BufferData(
			GraphicsBufferTarget::ShaderStorageBuffer,
			static_cast<GEsizeiptr>(instances_.size() * sizeof(GPUFoliageInstance)),
			instances_.empty() ? nullptr : instances_.data(),
			GraphicsBufferUsage::DynamicDraw);
		GraphicsAPI()->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0u);

		instancesDirty_ = false;
	}

	if (drawBuffersDirty_)
	{
		RefreshDrawBuffers();
	}
}

void GPUFoliageSystem::UpdateStaticMeshLOD(const Camera* activeCamera)
{
	if (!staticMesh_ || instances_.empty())
	{
		return;
	}

	float estimatedInstanceCoverage = 1.f;
	if (activeCamera)
	{
		const float communalAABBCoverage = activeCamera->GetAABBFrameCoverage(localAABB_, worldTransform_);
		estimatedInstanceCoverage = communalAABBCoverage / static_cast<float>(instances_.size());
	}

	const StaticMeshLOD* selectedStaticMesh = staticMesh_->GetLOD(
		static_cast<int>(staticMesh_->GetLODIndex(estimatedInstanceCoverage)));
	if (!selectedStaticMesh || selectedStaticMesh == staticMeshLOD_)
	{
		return;
	}

	staticMeshLOD_ = selectedStaticMesh;
	if (isInitialized_)
	{
		RefreshRenderBatches();
		RefreshDrawBuffers();
	}
}

void GPUFoliageSystem::RecalculateAABB()
{
	localAABB_ = Box();
	bool hasBounds = false;

	if (staticMesh_)
	{
		for (const GPUFoliageInstance& instance : instances_)
		{
			AddTransformedAABBToBounds(staticMesh_->GetAABB(), instance.transform, localAABB_, hasBounds);
		}
	}

	if (hasBounds)
	{
		localAABB_.CalculateSize();
	}
}

void GPUFoliageSystem::RenderMeshBatches(RenderPassType renderPassType, int& drawCount)
{
	if (instances_.empty() || renderBatches_.empty())
	{
		return;
	}

	GraphicsAPI()->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, ShaderBindingPoints::ShaderStorage::INSTANCE_DATA, instanceBufferId_);
	engine->GetRenderer()->BindStaticMeshBuffers();
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, drawIndirectBufferId_);

	for (std::uint32_t batchIndex = 0u; batchIndex < static_cast<std::uint32_t>(renderBatches_.size()); ++batchIndex)
	{
		const RenderBatch& renderBatch = renderBatches_[batchIndex];
		Shader* shader = GetBatchShader(renderBatch, renderPassType);
		if (!shader)
		{
			continue;
		}

		shader->Use();
		shader->SetMVP(worldTransform_);
		ApplyMaterialUniforms(renderBatch, shader, renderPassType);

		if (renderBatch.material && renderBatch.material->GetShadingModel() == MaterialShadingModel::TwoSided)
		{
			GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, false);
		}
		else
		{
			GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
		}

		const void* indirectCommandOffset = reinterpret_cast<const void*>(
			static_cast<std::uintptr_t>(batchIndex * sizeof(DrawElementsIndirectCommand)));
		GraphicsAPI()->DrawElementsIndirect(GraphicsPrimitive::Triangles, GraphicsDataType::UnsignedInt, indirectCommandOffset);
		++drawCount;
	}

	GraphicsAPI()->SetCapabilityEnabled(GraphicsCapability::CullFace, true);
	GraphicsAPI()->BindBuffer(GraphicsBufferTarget::DrawIndirectBuffer, 0u);
}

Shader* GPUFoliageSystem::CreateMeshShader(const RenderBatch& renderBatch, RenderPassType renderPassType) const
{
	Shader* shader = new Shader();
	AddMaterialTexturesToShader(renderBatch.material, shader, renderPassType);

	MaterialInitializationData materialInitializationData(renderBatch.material);
	MaterialInitializationData* materialInitializationDataPtr =
		TryBuildGPUFoliageMaterialInitializationData(renderBatch.material, materialInitializationData) ? &materialInitializationData : nullptr;

	shader->SetVertexShaderScript(
		ShaderBuilder::GetInstance()->GPUFoliageRenderPass_GetMeshVertexShaderScript(materialInitializationDataPtr, shader, renderPassType));
	if (renderPassType == RenderPassType::PointLightShadow)
	{
		shader->SetGeometryShaderScript(
			ShaderBuilder::GetInstance()->PointShadowPass_GetGeometryShaderScript(materialInitializationDataPtr, shader));
	}
	shader->SetFragmentShaderScript(
		ShaderBuilder::GetInstance()->GPUFoliageRenderPass_GetFragmentShaderScript(materialInitializationDataPtr, shader, renderPassType));

	shader->PreInit();
	shader->Init();
	if (renderPassType == RenderPassType::Forward && engine && engine->GetRenderer())
	{
		engine->GetRenderer()->BindShadowTextures(shader);
	}

	if (hasPostInitialized_)
	{
		shader->PostInit();
	}

	return shader;
}

Shader* GPUFoliageSystem::GetBatchShader(const RenderBatch& renderBatch, RenderPassType renderPassType) const
{
	switch (renderPassType)
	{
	case RenderPassType::Forward:
		return renderBatch.forwardShader;
	case RenderPassType::GeometryBuffer:
		return renderBatch.geometryBufferShader;
	case RenderPassType::PointLightShadow:
		return renderBatch.pointShadowShader;
	case RenderPassType::Shadow:
		return renderBatch.shadowShader;
	default:
		return nullptr;
	}
}

void GPUFoliageSystem::ApplyMaterialUniforms(const RenderBatch& renderBatch, Shader* shader, RenderPassType renderPassType) const
{
	if (!shader)
	{
		return;
	}

	const IMaterialBase* material = renderBatch.material;
	if (engine)
	{
		engine->SetShaderEngineVariables(shader);
	}

	shader->SetVector4(SHADER_VARIABLE_NAMES::MATERIAL::BASE_COLOR, material ? material->GetBaseColor() : Vector4(1.f));
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT_OCCLUSION, material ? material->GetAmbientOcclusion() : 1.f);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::METALLIC, material ? material->GetMetallic() : 0.f);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::ROUGHNESS, material ? material->GetRoughness() : 0.5f);
	shader->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::EMISIVE_COLOR, material ? material->GetEmissiveColor() : Vector3::ZeroVector);
	shader->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::TRANSLUCENCY, material ? material->GetTranslucency() : 0.f);
	shader->SetInt(SHADER_VARIABLE_NAMES::MATERIAL::SHADING_TYPE_ID, material ? static_cast<int>(material->GetShadingType()) : 0);

	if ((renderPassType == RenderPassType::Shadow || renderPassType == RenderPassType::PointLightShadow) &&
		engine && engine->GetRenderer() && engine->GetRenderer()->GetLightManager())
	{
		engine->GetRenderer()->GetLightManager()->SetShadowRenderPassShaderUniforms(shader);
	}

	if (renderPassType == RenderPassType::Forward && engine && engine->GetRenderer())
	{
		engine->GetRenderer()->SetLightUniforms(shader);
		if (material && material->GetUsesReflectionProbe())
		{
			engine->GetRenderer()->SetReflectionProbeUniforms(shader);
		}
	}
}
