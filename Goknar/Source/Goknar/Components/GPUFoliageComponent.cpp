#include "pch.h"

#include "GPUFoliageComponent.h"

#include "Goknar/Model/StaticMesh.h"

GPUFoliageComponent::GPUFoliageComponent(Component* parent) :
	Component(parent)
{
	EnsureFoliageSystem();
}

GPUFoliageComponent::~GPUFoliageComponent()
{
	DestroyFoliageSystem();
}

Component* GPUFoliageComponent::Clone() const
{
	GPUFoliageComponent* clonedComponent = new GPUFoliageComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (foliageSystem_)
	{
		clonedComponent->SetStaticMesh(foliageSystem_->GetStaticMesh());
		clonedComponent->SetInstances(foliageSystem_->GetInstances());
		clonedComponent->SetCastsShadow(foliageSystem_->GetCastsShadow());
	}

	return clonedComponent;
}

void GPUFoliageComponent::PreInit()
{
	Component::PreInit();
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		SyncTransformToSystem();
		foliageSystem_->PreInit();
	}
}

void GPUFoliageComponent::PostInit()
{
	if (foliageSystem_)
	{
		foliageSystem_->PostInit();
	}
	Component::PostInit();
}

void GPUFoliageComponent::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);
	if (foliageSystem_)
	{
		foliageSystem_->SetIsActive(isActive);
	}
}

void GPUFoliageComponent::DestroyInner()
{
	DestroyFoliageSystem();
	Component::DestroyInner();
}

void GPUFoliageComponent::SetStaticMesh(const StaticMesh* staticMesh)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->SetStaticMesh(staticMesh);
	}
}

const StaticMesh* GPUFoliageComponent::GetStaticMesh() const
{
	return foliageSystem_ ? foliageSystem_->GetStaticMesh() : nullptr;
}

void GPUFoliageComponent::SetInstances(const std::vector<GPUFoliageInstance>& instances, bool recalculateBounds)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->SetInstances(instances, recalculateBounds);
	}
}

void GPUFoliageComponent::SetInstanceTransformations(const std::vector<Matrix>& instanceTransformations, bool recalculateBounds)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->SetInstanceTransformations(instanceTransformations, recalculateBounds);
	}
}

void GPUFoliageComponent::AddInstance(const GPUFoliageInstance& instance, bool recalculateBounds)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->AddInstance(instance, recalculateBounds);
	}
}

void GPUFoliageComponent::AddInstanceTransformation(const Matrix& instanceTransformation, bool recalculateBounds)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->AddInstanceTransformation(instanceTransformation, recalculateBounds);
	}
}

void GPUFoliageComponent::ClearInstances()
{
	if (foliageSystem_)
	{
		foliageSystem_->ClearInstances();
	}
}

const std::vector<GPUFoliageInstance>& GPUFoliageComponent::GetInstances() const
{
	static const std::vector<GPUFoliageInstance> emptyInstances;
	return foliageSystem_ ? foliageSystem_->GetInstances() : emptyInstances;
}

std::size_t GPUFoliageComponent::GetInstanceCount() const
{
	return foliageSystem_ ? foliageSystem_->GetInstanceCount() : 0u;
}

void GPUFoliageComponent::SetCastsShadow(bool castsShadow)
{
	EnsureFoliageSystem();
	if (foliageSystem_)
	{
		foliageSystem_->SetCastsShadow(castsShadow);
	}
}

bool GPUFoliageComponent::GetCastsShadow() const
{
	return foliageSystem_ ? foliageSystem_->GetCastsShadow() : false;
}

void GPUFoliageComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();
	SyncTransformToSystem();
}

void GPUFoliageComponent::EnsureFoliageSystem()
{
	if (!foliageSystem_)
	{
		foliageSystem_ = new GPUFoliageSystem();
		foliageSystem_->SetIsActive(GetIsActive());
		SyncTransformToSystem();
	}
}

void GPUFoliageComponent::DestroyFoliageSystem()
{
	delete foliageSystem_;
	foliageSystem_ = nullptr;
}

void GPUFoliageComponent::SyncTransformToSystem() const
{
	if (foliageSystem_)
	{
		foliageSystem_->SetWorldTransform(GetComponentToWorldTransformationMatrix());
	}
}
