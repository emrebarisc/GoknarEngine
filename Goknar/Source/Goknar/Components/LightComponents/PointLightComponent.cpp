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

Component* PointLightComponent::Clone() const
{
	PointLightComponent* clonedComponent = new PointLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (pointLight_ && clonedComponent->pointLight_)
	{
		clonedComponent->pointLight_->SetPosition(pointLight_->GetPosition());
		clonedComponent->pointLight_->SetColor(pointLight_->GetColor());
		clonedComponent->pointLight_->SetIntensity(pointLight_->GetIntensity());
		clonedComponent->pointLight_->SetShadowIntensity(pointLight_->GetShadowIntensity());
		clonedComponent->pointLight_->SetLightMobility(pointLight_->GetLightMobility());
		clonedComponent->pointLight_->SetName(pointLight_->GetName());
		clonedComponent->pointLight_->SetIsShadowEnabled(pointLight_->GetIsShadowEnabled());
		clonedComponent->pointLight_->SetShadowWidth(pointLight_->GetShadowWidth());
		clonedComponent->pointLight_->SetShadowHeight(pointLight_->GetShadowHeight());
		clonedComponent->pointLight_->SetRadius(pointLight_->GetRadius());
	}

	return clonedComponent;
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
