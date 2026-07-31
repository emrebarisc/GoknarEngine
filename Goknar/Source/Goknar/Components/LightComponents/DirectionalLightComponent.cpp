#include "DirectionalLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/DirectionalLight.h"

DirectionalLightComponent::DirectionalLightComponent(Component* parentComponent) :
	Component(parentComponent),
	directionalLight_(new DirectionalLight())
{
}

DirectionalLightComponent::~DirectionalLightComponent()
{
}

void DirectionalLightComponent::Destroy()
{
	Component::Destroy();
}

Component* DirectionalLightComponent::Clone() const
{
	DirectionalLightComponent* clonedComponent = new DirectionalLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (directionalLight_ && clonedComponent->directionalLight_)
	{
		clonedComponent->directionalLight_->SetPosition(directionalLight_->GetPosition());
		clonedComponent->directionalLight_->SetDirection(directionalLight_->GetDirection());
		clonedComponent->directionalLight_->SetColor(directionalLight_->GetColor());
		clonedComponent->directionalLight_->SetIntensity(directionalLight_->GetIntensity());
		clonedComponent->directionalLight_->SetShadowIntensity(directionalLight_->GetShadowIntensity());
		clonedComponent->directionalLight_->SetLightMobility(directionalLight_->GetLightMobility());
		clonedComponent->directionalLight_->SetName(directionalLight_->GetName());
		clonedComponent->directionalLight_->SetIsShadowEnabled(directionalLight_->GetIsShadowEnabled());
		clonedComponent->directionalLight_->SetShadowWidth(directionalLight_->GetShadowWidth());
		clonedComponent->directionalLight_->SetShadowHeight(directionalLight_->GetShadowHeight());
	}

	return clonedComponent;
}

void DirectionalLightComponent::DestroyInner()
{
	Component::DestroyInner();
	delete directionalLight_;
}

void DirectionalLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	directionalLight_->SetPosition(worldPosition_);
	directionalLight_->SetDirection(GetWorldForwardVector());
}
