#ifndef __SPOTLIGHTCOMPONENT_H__
#define __SPOTLIGHTCOMPONENT_H__

#include "Core.h"
#include "LightComponent.h"

#include "Goknar/Lights/SpotLight.h"

class GOKNAR_API SpotLightComponent : public LightComponent<SpotLight>
{
public:
	SpotLightComponent(Component* parentComponent);
	virtual ~SpotLightComponent();

	Component* Clone() const override;

protected:
	virtual void UpdateComponentToWorldTransformationMatrix() override;

private:
};

#endif
