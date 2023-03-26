#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/ObjectBase.h"

MeshComponent::MeshComponent(Component* parent, MeshInstance* meshInstance) :
	RenderComponent(parent),
	meshInstance_(meshInstance)
{
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

void MeshComponent::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);
	meshInstance_->SetIsRendered(isActive);
}