#include "DirectionalLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/DirectionalLight.h"

DirectionalLightComponent::DirectionalLightComponent(Component* parentComponent) :
	LightComponent<DirectionalLight>(parentComponent)
{
}

DirectionalLightComponent::~DirectionalLightComponent()
{
}

Component* DirectionalLightComponent::Clone() const
{
	DirectionalLightComponent* clonedComponent = new DirectionalLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (light_ && clonedComponent->light_)
	{
		clonedComponent->light_->SetPosition(light_->GetPosition());
		clonedComponent->light_->SetDirection(light_->GetDirection());
		clonedComponent->light_->SetColor(light_->GetColor());
		clonedComponent->light_->SetIntensity(light_->GetIntensity());
		clonedComponent->light_->SetShadowIntensity(light_->GetShadowIntensity());
		clonedComponent->light_->SetLightMobility(light_->GetLightMobility());
		clonedComponent->light_->SetName(light_->GetName());
		clonedComponent->light_->SetIsShadowEnabled(light_->GetIsShadowEnabled());
		clonedComponent->light_->SetShadowWidth(light_->GetShadowWidth());
		clonedComponent->light_->SetShadowHeight(light_->GetShadowHeight());
	}

	return clonedComponent;
}

void DirectionalLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	LightComponent<DirectionalLight>::UpdateComponentToWorldTransformationMatrix();

	if (!isInitialized_)
	{
		return;
	}

	light_->SetPosition(worldPosition_);
	light_->SetDirection(GetWorldForwardVector());
}
