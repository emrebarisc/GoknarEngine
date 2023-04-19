#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "RenderComponent.h"

#include "Goknar/Core.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/ObjectBase.h"

class MeshUnit;
class ObjectBase;

template<class MeshType, class MeshInstanceType>
class GOKNAR_API MeshComponent : public RenderComponent
{
public:
	MeshComponent() = delete;
	inline virtual ~MeshComponent();

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
MeshComponent<MeshType, MeshInstanceType>::~MeshComponent()
{

}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::Destroy()
{
	if (meshInstance_)
	{
		meshInstance_->Destroy();
	}

	Component::Destroy();
}

template<class MeshType, class MeshInstanceType>
void MeshComponent<MeshType, MeshInstanceType>::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);
	meshInstance_->SetIsRendered(isActive);
}

#endif