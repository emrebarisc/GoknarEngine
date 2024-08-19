#include "PointLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/PointLight.h"

PointLightComponent::PointLightComponent(Component* parentComponent) :
	Component(parentComponent),
	pointLight_(new PointLight())
{
}

PointLightComponent::~PointLightComponent()
{
}

void PointLightComponent::Destroy()
{
	Component::Destroy();
}

void PointLightComponent::DestroyInner()
{
	Component::DestroyInner();
	delete pointLight_;
}

void PointLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	Vector3 lightPosition = worldRotation_.GetMatrix() * relativeRotation_.GetMatrix() * Vector4 { relativePosition_, 1.f };
	lightPosition = worldPosition_ - relativePosition_ + lightPosition;

	pointLight_->SetPosition(lightPosition);
}