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

	pointLight_->SetPosition(worldPosition_);
}