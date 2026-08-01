#include "SpotLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/SpotLight.h"

SpotLightComponent::SpotLightComponent(Component* parentComponent) :
	Component(parentComponent),
	spotLight_(new SpotLight())
{
}

SpotLightComponent::~SpotLightComponent()
{
}

void SpotLightComponent::Destroy()
{
	Component::Destroy();
}

Component* SpotLightComponent::Clone() const
{
	SpotLightComponent* clonedComponent = new SpotLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (spotLight_ && clonedComponent->spotLight_)
	{
		clonedComponent->spotLight_->SetPosition(spotLight_->GetPosition());
		clonedComponent->spotLight_->SetDirection(spotLight_->GetDirection());
		clonedComponent->spotLight_->SetFalloffAngle(spotLight_->GetFalloffAngle());
		clonedComponent->spotLight_->SetCoverageAngle(spotLight_->GetCoverageAngle());
		clonedComponent->spotLight_->SetColor(spotLight_->GetColor());
		clonedComponent->spotLight_->SetIntensity(spotLight_->GetIntensity());
		clonedComponent->spotLight_->SetShadowIntensity(spotLight_->GetShadowIntensity());
		clonedComponent->spotLight_->SetLightMobility(spotLight_->GetLightMobility());
		clonedComponent->spotLight_->SetName(spotLight_->GetName());
		clonedComponent->spotLight_->SetIsShadowEnabled(spotLight_->GetIsShadowEnabled());
		clonedComponent->spotLight_->SetShadowWidth(spotLight_->GetShadowWidth());
		clonedComponent->spotLight_->SetShadowHeight(spotLight_->GetShadowHeight());
		clonedComponent->spotLight_->SetRadius(spotLight_->GetRadius());
	}

	return clonedComponent;
}

void SpotLightComponent::DestroyInner()
{
	Component::DestroyInner();
	delete spotLight_;
}

void SpotLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();

	spotLight_->SetPosition(GetWorldPosition());
	spotLight_->SetDirection(GetWorldForwardVector());
}
