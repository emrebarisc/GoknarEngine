#ifndef __STATIC_MESH_PARTICLE_SYSTEM_H__
#define __STATIC_MESH_PARTICLE_SYSTEM_H__

#include "ParticleSystemBase.h"

#include <vector>

class IMaterialBase;
class MeshGeometry;
class Shader;
class StaticMeshLOD;

class GOKNAR_API StaticMeshParticleSystem : public ParticleSystemBase
{
public:
	explicit StaticMeshParticleSystem(const GPUParticleSystemDesc& desc = GPUParticleSystemDesc());
	~StaticMeshParticleSystem() override;

	std::uint32_t Render(
		const Camera* activeCamera,
		ParticleRenderStage renderStage = ParticleRenderStage::All) const override;

	void SetStaticMesh(const StaticMeshLOD* staticMesh);
	const StaticMeshLOD* GetStaticMesh() const
	{
		return staticMesh_;
	}

protected:
	void CreateRenderResources() override;
	void DestroyRenderResources() override;
	void OnInit() override;
	void OnPostInit() override;
	void RecreateDrawIndirectBuffer() override;
	void DispatchFinalizePass() const override;

private:
	struct StaticMeshSubmeshRenderData
	{
		const MeshGeometry* meshUnit{ nullptr };
		const IMaterialBase* material{ nullptr };
		Shader* renderShader{ nullptr };
		std::uint32_t indexCount{ 0u };
		std::uint32_t firstIndex{ 0u };
		std::int32_t baseVertex{ 0 };
	};

	Shader* CreateRenderShaderForMaterial(const IMaterialBase* material) const;
	void RefreshStaticMeshSubmeshRenderData();

	const StaticMeshLOD* staticMesh_{ nullptr };
	std::vector<StaticMeshSubmeshRenderData> staticMeshSubmeshRenderData_{};
};

#endif
