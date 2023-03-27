#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "RenderComponent.h"

#include "Goknar/Core.h"
#include "Goknar/Model/MeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/ObjectBase.h"

class MeshUnit;
class ObjectBase;

template<class InstanceType>
class GOKNAR_API MeshComponent : public RenderComponent
{
public:
	MeshComponent() = delete;
	virtual ~MeshComponent();

	void Destroy() override;

	virtual void SetMesh(InstanceType* mesh) = 0;

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

	void SetIsActive(bool isRendered);

	MeshInstance<InstanceType>* GetMeshInstance() const
	{
		return meshInstance_;
	}
protected:
	MeshComponent(Component* parent, MeshInstance<InstanceType>* meshInstance);
	inline void UpdateRelativeTransformationMatrix() override
	{
		RenderComponent::UpdateRelativeTransformationMatrix();

		meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
	}

	MeshInstance<InstanceType>* meshInstance_;
private:
};

template<class InstanceType>
MeshComponent<InstanceType>::MeshComponent(Component* parent, MeshInstance<InstanceType>* meshInstance) :
	RenderComponent(parent),
	meshInstance_(meshInstance)
{
}

template<class InstanceType>
MeshComponent<InstanceType>::~MeshComponent()
{

}

template<class InstanceType>
void MeshComponent<InstanceType>::Destroy()
{
	if (meshInstance_)
	{
		meshInstance_->Destroy();
	}
	delete this;
}

template<class InstanceType>
void MeshComponent<InstanceType>::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

template<class InstanceType>
void MeshComponent<InstanceType>::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);
	meshInstance_->SetIsRendered(isActive);
}

#endif