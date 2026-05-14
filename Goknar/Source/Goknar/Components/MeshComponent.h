#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "RenderComponent.h"

#include "Goknar/Core.h"
#include "Goknar/Model/IMeshInstance.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/ObjectBase.h"

class MeshUnit;
class ObjectBase;

template<class MeshType, class MeshInstanceType>
class GOKNAR_API MeshComponent : public RenderComponent
{
public:
	MeshComponent(const MeshComponent&) = delete;
	MeshComponent() = delete;
	inline virtual ~MeshComponent();

	inline virtual void PreInit() override;
	inline virtual void Init() override;
	inline virtual void PostInit() override; 

	inline void Destroy() override;

	inline virtual void SetMesh(MeshType* mesh) = 0;

	inline const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	inline void SetIsActive(bool isRendered);

	inline MeshInstanceType* GetMeshInstance() const
	{
		return meshInstance_;
	}
protected:
	inline MeshComponent(Component* parent);
	inline void DestroyInner() override;
	inline void CopyMeshComponentValuesTo(MeshComponent<MeshType, MeshInstanceType>* meshComponent) const;

	MeshInstanceType* meshInstance_;
private:
};

template<class MeshType, class MeshInstanceType>
MeshComponent<MeshType, MeshInstanceType>::MeshComponent(Component* parent) :
	RenderComponent(parent),
	meshInstance_(new MeshInstanceType(this))
{
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::PreInit()
{
	Component::PreInit();

	meshInstance_->PreInit();
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::Init()
{
	Component::Init();

	meshInstance_->Init();
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::PostInit()
{
	Component::PostInit();

	meshInstance_->PostInit();
}

template<class MeshType, class MeshInstanceType>
MeshComponent<MeshType, MeshInstanceType>::~MeshComponent()
{

}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::DestroyInner()
{
	Component::DestroyInner();
	
	if (meshInstance_)
	{
		meshInstance_->Destroy();
	}
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::Destroy()
{
	Component::Destroy();
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);
	meshInstance_->SetIsRendered(isActive);
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::CopyMeshComponentValuesTo(MeshComponent<MeshType, MeshInstanceType>* meshComponent) const
{
	if (!meshComponent)
	{
		return;
	}

	CopyValuesTo(meshComponent);

	if (!meshInstance_ || !meshComponent->meshInstance_)
	{
		return;
	}

	MeshType* mesh = meshInstance_->GetMesh();
	if (mesh)
	{
		meshComponent->SetMesh(mesh);
	}

	meshComponent->meshInstance_->SetRenderMask(meshInstance_->GetRenderMask());
	meshComponent->meshInstance_->SetIsRendered(meshInstance_->GetIsRendered());
	meshComponent->meshInstance_->SetIsCastingShadow(meshInstance_->GetIsCastingShadow());

	const std::vector<MaterialInstance*>& materials = meshInstance_->GetMaterials();
	for (int materialIndex = 0; materialIndex < (int)materials.size(); ++materialIndex)
	{
		MaterialInstance* sourceMaterialInstance = materials[materialIndex];
		if (!sourceMaterialInstance)
		{
			continue;
		}

		Material* parentMaterial = sourceMaterialInstance->GetParentMaterial();
		if (!parentMaterial)
		{
			continue;
		}

		MaterialInstance* clonedMaterialInstance = MaterialInstance::Create(parentMaterial);
		clonedMaterialInstance->SetName(sourceMaterialInstance->GetName());
		clonedMaterialInstance->SetBaseColor(sourceMaterialInstance->GetBaseColor());
		clonedMaterialInstance->SetAmbientOcclusion(sourceMaterialInstance->GetAmbientOcclusion());
		clonedMaterialInstance->SetMetallic(sourceMaterialInstance->GetMetallic());
		clonedMaterialInstance->SetRoughness(sourceMaterialInstance->GetRoughness());
		clonedMaterialInstance->SetEmissiveColor(sourceMaterialInstance->GetEmissiveColor());
		clonedMaterialInstance->SetTranslucency(sourceMaterialInstance->GetTranslucency());
		clonedMaterialInstance->SetBlendModel(sourceMaterialInstance->GetBlendModel());
		clonedMaterialInstance->SetShadingModel(sourceMaterialInstance->GetShadingModel());
		clonedMaterialInstance->SetUsesReflectionProbe(sourceMaterialInstance->GetUsesReflectionProbe());

		meshComponent->meshInstance_->SetMaterial(materialIndex, clonedMaterialInstance);
	}
}

#endif
