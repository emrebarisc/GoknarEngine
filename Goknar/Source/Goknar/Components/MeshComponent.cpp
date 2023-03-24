#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/ObjectBase.h"

MeshComponent::MeshComponent(ObjectBase* parent, MeshInstance* meshInstance) :
	RenderComponent(parent),
	meshInstance_(meshInstance)
{
	WorldTransformationMatrixIsUpdated(parent->GetWorldTransformationMatrix());
}

MeshComponent::~MeshComponent()
{
	
}

void MeshComponent::Destroy()
{
	if (meshInstance_)
	{
		meshInstance_->Destroy();
	}
	delete this;
}

void MeshComponent::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

void MeshComponent::SetIsRendered(bool isRendered)
{
	isRendered_ = isRendered;
	meshInstance_->SetIsRendered(isRendered_);
}