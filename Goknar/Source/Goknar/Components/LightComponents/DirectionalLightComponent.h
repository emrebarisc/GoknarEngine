#ifndef __DIRECTIONALLIGHTCOMPONENT_H__
#define __DIRECTIONALLIGHTCOMPONENT_H__

#include "Core.h"
#include "LightComponent.h"

#include "Goknar/Lights/DirectionalLight.h"

class GOKNAR_API DirectionalLightComponent : public LightComponent<DirectionalLight>
{
public:
	DirectionalLightComponent(Component* parentComponent);
	virtual ~DirectionalLightComponent();

	Component* Clone() const override;

protected:
	virtual void UpdateComponentToWorldTransformationMatrix() override;

private:
};

#endif
