#include "CameraComponent.h"

#include "ObjectBase.h"

#include "Goknar/Camera.h"

CameraComponent::CameraComponent(Component* parentComponent) :
	Component(parentComponent),
	camera_(new Camera())
{
}

void CameraComponent::Destroy()
{
	Component::Destroy();
}

Component* CameraComponent::Clone() const
{
	CameraComponent* clonedComponent = new CameraComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);
	clonedComponent->cameraFollowsComponentRotation_ = cameraFollowsComponentRotation_;

	if (camera_ && clonedComponent->camera_)
	{
		clonedComponent->camera_->SetPosition(camera_->GetPosition(), false);
		clonedComponent->camera_->SetVectors(camera_->GetForwardVector(), camera_->GetLeftVector(), camera_->GetUpVector(), false);
		clonedComponent->camera_->SetImageWidth(camera_->GetImageWidth());
		clonedComponent->camera_->SetImageHeight(camera_->GetImageHeight());
		clonedComponent->camera_->SetNearDistance(camera_->GetNearDistance());
		clonedComponent->camera_->SetFarDistance(camera_->GetFarDistance());
		clonedComponent->camera_->SetNearPlane(camera_->GetNearPlane());
		clonedComponent->camera_->SetProjection(camera_->GetProjection());
		clonedComponent->camera_->SetTemporalJitter(camera_->GetTemporalJitter());
		clonedComponent->camera_->SetCameraType(camera_->GetCameraType());
		clonedComponent->camera_->SetRenderMask(camera_->GetRenderMask());
		clonedComponent->camera_->Update();
	}

	return clonedComponent;
}

void CameraComponent::DestroyInner()
{
	Component::DestroyInner();
	camera_->Destroy();
}

void CameraComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	if (cameraFollowsComponentRotation_)
	{
		camera_->SetPosition(worldPosition_);
		camera_->SetVectors(GetWorldForwardVector(), GetWorldLeftVector(), GetWorldUpVector());
	}
	else
	{
		camera_->SetPosition(owner_->GetWorldPosition() + GetRelativePosition());
		camera_->SetVectors(GetRelativeForwardVector(), GetRelativeLeftVector(), GetRelativeUpVector());
	}
}

void CameraComponent::UpdateChildrenComponentToWorldTransformations()
{
	Component::UpdateChildrenComponentToWorldTransformations();
}
