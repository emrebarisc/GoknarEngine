#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/RenderComponent.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialInstance.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/Mesh.h"

#include "Goknar/Renderer/Renderer.h"

template<class MeshType>
class GOKNAR_API IMeshInstance
{
public:
	IMeshInstance() = delete;

	inline IMeshInstance(RenderComponent* parentComponent) :
		parentComponent_(parentComponent)
	{
	}

	inline virtual ~IMeshInstance()
	{
		for (MaterialInstance* materialInstance : materials_)
		{
			materialInstance->Destroy();
		}

		materials_.clear();
	}

	inline void PreInit();
	inline void Init();
	inline void PostInit();

	const RenderComponent* GetParentComponent() const
	{
		return parentComponent_;
	}

	inline int GetInstanceID() const
	{
		return instanceID_;
	}

	inline void SetInstanceID(int instanceID)
	{
		instanceID_ = instanceID;
	}

	inline virtual void SetMesh(MeshType* mesh);

	inline MeshType* GetMesh() const
	{
		return mesh_;
	}

	inline void SetMaterial(int index, MaterialInstance* material);

	inline virtual void PreRender(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0);
	inline virtual void Render(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0);

	inline void SetIsRendered(bool isRendered)
	{
		isRendered_ = isRendered;
	}

	inline bool GetIsRendered() const
	{
		return isRendered_;
	}

	inline void SetIsCastingShadow(bool isCastingShadow)
	{
		isCastingShadow_ = isCastingShadow;
	}

	inline bool GetIsCastingShadow() const
	{
		return isCastingShadow_;
	}

	unsigned int GetRenderMask() const
	{
		return renderMask_;
	}

	void SetRenderMask(unsigned int renderMask)
	{
		renderMask_ = renderMask;
	}

	int GetForcedLODIndex() const
	{
		return forcedLODIndex_;
	}

	void SetForcedLODIndex(int forcedLODIndex)
	{
		forcedLODIndex_ = forcedLODIndex;
	}

	const std::vector<MaterialInstance*>& GetMaterials() const
	{
		return materials_;
	}

	IMaterialBase* GetMaterial(int index) const
	{
		GOKNAR_CORE_ASSERT(index < materials_.size(), "Requested material index is greater than material size");

		IMaterialBase* material = materials_[index];

		if (!material)
		{
			material = mesh_->GetLOD(0)->GetMesh(index)->GetMaterial();
		}

		return material;
	}

	inline virtual void Destroy();

protected:
	virtual void AddMeshInstanceToRenderer() = 0;
	virtual void RemoveMeshInstanceFromRenderer() = 0;

	MeshType* mesh_{ nullptr };

	RenderComponent* parentComponent_{ nullptr };

	std::vector<MaterialInstance*> materials_{ nullptr };
private:
	unsigned int renderMask_{ 0b1 };

	int instanceID_{ lastComponentId_++ };
	int forcedLODIndex_{ -1 };

	static int lastComponentId_;

	bool isRendered_{ true };
	bool isCastingShadow_{ true };
	bool isInitialized_{ false };
};

template<class MeshType>
int IMeshInstance<MeshType>::lastComponentId_ = 0;

template<class MeshType>
inline void IMeshInstance<MeshType>::PreInit()
{
	if (mesh_)
	{
		AddMeshInstanceToRenderer();
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::Init()
{
}

template<class MeshType>
inline void IMeshInstance<MeshType>::PostInit()
{
	isInitialized_ = true;
}

template<class MeshType>
inline void IMeshInstance<MeshType>::SetMaterial(int index, MaterialInstance* material)
{
	if (materials_[index] == material)
	{
		return;
	}

	const IMaterialBase* previousMaterial = materials_[index] ? static_cast<IMaterialBase*>(materials_[index]) : static_cast<IMaterialBase*>(mesh_->GetLOD(0)->GetMesh(index)->GetMaterial());
	const IMaterialBase* nextMaterial = material ? static_cast<IMaterialBase*>(material) : static_cast<IMaterialBase*>(mesh_->GetLOD(0)->GetMesh(index)->GetMaterial());
	const bool refreshInstanceOnRenderer =
		previousMaterial != nullptr &&
		nextMaterial != nullptr &&
		previousMaterial->GetBlendModel() != nextMaterial->GetBlendModel();

	if (materials_[index])
	{
		materials_[index]->Destroy();
		materials_[index] = nullptr;
	}

	materials_[index] = material;

	if (refreshInstanceOnRenderer)
	{
		RemoveMeshInstanceFromRenderer();
		AddMeshInstanceToRenderer();
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::PreRender(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	auto LOD = mesh_->GetLOD(LODIndex);
	auto& subMeshes = LOD->GetSubMeshes();

	IMaterialBase* material = materials_[subMeshIndex];

	if (!material)
	{
		material = subMeshes[subMeshIndex]->GetMaterial();
	}

	if (material)
	{
		material->Use(renderPassType);
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	const auto& subMeshes = mesh_->GetLOD(LODIndex)->GetSubMeshes();

	IMaterialBase* material = materials_[subMeshIndex];

	if (!material)
	{
		material = subMeshes[subMeshIndex]->GetMaterial();
	}

	if (material)
	{
		material->SetShaderVariables(renderPassType, parentComponent_->GetComponentToWorldTransformationMatrix());
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::SetMesh(MeshType* mesh)
{
	if (mesh_ != nullptr)
	{
		RemoveMeshInstanceFromRenderer();
	}

	mesh_ = mesh;

	if (mesh_->GetLODCount() == 1)
	{
		SetForcedLODIndex(0);
	}

	materials_.resize(mesh_->GetLOD(0)->GetSubMeshes().size(), nullptr);

	if (!isInitialized_)
	{
		PreInit();
		Init();
		PostInit();
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::Destroy()
{
	if (mesh_)
	{
		RemoveMeshInstanceFromRenderer();
	}

	for (MaterialInstance* materialInstance : materials_)
	{
		if (!materialInstance)
		{
			continue;
		}

		materialInstance->Destroy();
	}

	materials_.clear();

	delete this;
}

#endif
