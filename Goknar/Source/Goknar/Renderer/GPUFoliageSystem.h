#ifndef __GPUFOLIAGESYSTEM_H__
#define __GPUFOLIAGESYSTEM_H__

#include "Goknar/Core.h"
#include "Goknar/Geometry/Box.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Types.h"

#include <cstdint>
#include <vector>

class Camera;
class IMaterialBase;
class MeshGeometry;
class Shader;

struct GOKNAR_API GPUFoliageInstance
{
	Matrix transform{ Matrix::IdentityMatrix };
	Vector4 color{ Vector4(1.f) };
};

class GOKNAR_API GPUFoliageSystem
{
public:
	GPUFoliageSystem();
	GPUFoliageSystem(const GPUFoliageSystem&) = delete;
	GPUFoliageSystem& operator=(const GPUFoliageSystem&) = delete;
	~GPUFoliageSystem();

	void PreInit();
	void PostInit();

	void SetStaticMesh(const StaticMesh* staticMesh);
	const StaticMesh* GetStaticMesh() const
	{
		return staticMesh_;
	}

	void SetInstances(const std::vector<GPUFoliageInstance>& instances, bool recalculateBounds = true);
	void SetInstanceTransformations(const std::vector<Matrix>& instanceTransformations, bool recalculateBounds = true);
	void AddInstance(const GPUFoliageInstance& instance, bool recalculateBounds = true);
	void AddInstanceTransformation(const Matrix& instanceTransformation, bool recalculateBounds = true);
	void ClearInstances();

	const std::vector<GPUFoliageInstance>& GetInstances() const
	{
		return instances_;
	}

	std::size_t GetInstanceCount() const
	{
		return instances_.size();
	}

	void SetWorldTransform(const Matrix& worldTransform)
	{
		worldTransform_ = worldTransform;
	}

	const Matrix& GetWorldTransform() const
	{
		return worldTransform_;
	}

	const Box& GetLocalAABB() const
	{
		return localAABB_;
	}

	void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetIsActive() const
	{
		return isActive_;
	}

	void SetCastsShadow(bool castsShadow)
	{
		castsShadow_ = castsShadow;
	}

	bool GetCastsShadow() const
	{
		return castsShadow_;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	int Render(const Camera* activeCamera, RenderPassType renderPassType, bool skipFrustumCulling);

private:
	struct DrawElementsIndirectCommand
	{
		std::uint32_t count{ 0u };
		std::uint32_t instanceCount{ 0u };
		std::uint32_t firstIndex{ 0u };
		std::int32_t baseVertex{ 0 };
		std::uint32_t baseInstance{ 0u };
	};

	struct RenderBatch
	{
		const MeshGeometry* meshUnit{ nullptr };
		const IMaterialBase* material{ nullptr };
		Shader* forwardShader{ nullptr };
		Shader* geometryBufferShader{ nullptr };
		Shader* shadowShader{ nullptr };
		Shader* pointShadowShader{ nullptr };
		DrawElementsIndirectCommand drawCommand{};
	};

	void RegisterToRenderer();
	void UnregisterFromRenderer();
	void CreateBuffers();
	void DestroyBuffers();
	void RefreshRenderBatches();
	void DestroyRenderBatches();
	void RefreshDrawBuffers();
	void UploadInstanceBuffers();
	void UpdateStaticMeshLOD(const Camera* activeCamera);
	void RecalculateAABB();
	void RenderMeshBatches(RenderPassType renderPassType, int& drawCount);

	Shader* CreateMeshShader(const RenderBatch& renderBatch, RenderPassType renderPassType) const;
	Shader* GetBatchShader(const RenderBatch& renderBatch, RenderPassType renderPassType) const;
	void ApplyMaterialUniforms(const RenderBatch& renderBatch, Shader* shader, RenderPassType renderPassType) const;

	const StaticMesh* staticMesh_{ nullptr };
	const StaticMeshLOD* staticMeshLOD_{ nullptr };
	std::vector<GPUFoliageInstance> instances_;
	std::vector<RenderBatch> renderBatches_;

	Matrix worldTransform_{ Matrix::IdentityMatrix };
	Box localAABB_{};

	GEuint instanceBufferId_{ 0u };
	GEuint drawIndirectBufferId_{ 0u };

	bool isActive_{ true };
	bool castsShadow_{ true };
	bool isInitialized_{ false };
	bool hasPostInitialized_{ false };
	bool isRegistered_{ false };
	bool instancesDirty_{ true };
	bool drawBuffersDirty_{ true };
};

#endif
