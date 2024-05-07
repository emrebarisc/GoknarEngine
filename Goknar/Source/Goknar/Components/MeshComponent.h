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

#endif