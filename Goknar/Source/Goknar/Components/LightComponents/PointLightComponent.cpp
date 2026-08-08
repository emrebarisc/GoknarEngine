#include "PointLightComponent.h"

#include "ObjectBase.h"

#include "Goknar/Lights/PointLight.h"

PointLightComponent::PointLightComponent(Component* parentComponent) :
	LightComponent<PointLight>(parentComponent)
{
}

PointLightComponent::~PointLightComponent()
{
}

Component* PointLightComponent::Clone() const
{
	PointLightComponent* clonedComponent = new PointLightComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	if (light_ && clonedComponent->light_)
	{
		clonedComponent->light_->SetPosition(light_->GetPosition());
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

void PointLightComponent::UpdateComponentToWorldTransformationMatrix()
{
	LightComponent<PointLight>::UpdateComponentToWorldTransformationMatrix();

	if (!isInitialized_)
	{
		return;
	}

	light_->SetPosition(worldPosition_);
}
