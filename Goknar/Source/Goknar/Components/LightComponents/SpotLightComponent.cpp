#include "SpotLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/SpotLight.h"

SpotLightComponent::SpotLightComponent(Component* parentComponent) :
	LightComponent<SpotLight>(parentComponent)
{
}

SpotLightComponent::~SpotLightComponent()
{
}

Component* SpotLightComponent::Clone() const
{
	SpotLightComponent* clonedComponent = new SpotLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (light_ && clonedComponent->light_)
	{
		clonedComponent->light_->SetPosition(light_->GetPosition());
		clonedComponent->light_->SetDirection(light_->GetDirection());
		clonedComponent->light_->SetFalloffAngle(light_->GetFalloffAngle());
		clonedComponent->light_->SetCoverageAngle(light_->GetCoverageAngle());
		clonedComponent->light_->SetColor(light_->GetColor());
		clonedComponent->light_->SetIntensity(light_->GetIntensity());
		clonedComponent->light_->SetShadowIntensity(light_->GetShadowIntensity());
		clonedComponent->light_->SetLightMobility(light_->GetLightMobility());
		clonedComponent->light_->SetName(light_->GetName());
		clonedComponent->light_->SetIsShadowEnabled(light_->GetIsShadowEnabled());
		clonedComponent->light_->SetShadowWidth(light_->GetShadowWidth());
		clonedComponent->light_->SetShadowHeight(light_->GetShadowHeight());
		clonedComponent->light_->SetRadius(light_->GetRadius());
	}

	return clonedComponent;
}

void SpotLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	LightComponent<SpotLight>::UpdateComponentToWorldTransformationMatrix();

	if (!isInitialized_)
	{
		return;
	}

	light_->SetPosition(GetWorldPosition());
	light_->SetDirection(GetWorldForwardVector());
}
