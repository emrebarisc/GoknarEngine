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

void CameraComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	camera_->SetPosition(worldPosition_, false);
	camera_->SetVectors(GetForwardVector(), GetLeftVector(), GetUpVector(), true);
}

void CameraComponent::UpdateChildrenComponentToWorldTransformations()
{
	Component::UpdateChildrenComponentToWorldTransformations();
}
