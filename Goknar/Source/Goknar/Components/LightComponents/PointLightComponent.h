#ifndef __POINTLIGHTCOMPONENT_H__
#define __POINTLIGHTCOMPONENT_H__

#include "Core.h"
#include "LightComponent.h"

#include "Goknar/Lights/PointLight.h"

class GOKNAR_API PointLightComponent : public LightComponent<PointLight>
{
public:
	PointLightComponent(Component* parentComponent);
	virtual ~PointLightComponent();

	Component* Clone() const override;

protected:
	virtual void UpdateComponentToWorldTransformationMatrix() override;

private:
};

#endif
