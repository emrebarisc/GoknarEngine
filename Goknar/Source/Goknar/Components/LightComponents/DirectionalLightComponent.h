#ifndef __DIRECTIONALLIGHTCOMPONENT_H__
#define __DIRECTIONALLIGHTCOMPONENT_H__

#include "Core.h"
#include "Goknar/Components/Component.h"

class DirectionalLight;

class GOKNAR_API DirectionalLightComponent : public Component
{
public:
	DirectionalLightComponent(Component* parentComponent);
	virtual ~DirectionalLightComponent();

	virtual void Destroy() override;
	Component* Clone() const override;

	DirectionalLight* GetDirectionalLight() const
	{
		return directionalLight_;
	}

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();

private:
	virtual void DestroyInner() override;

	DirectionalLight* directionalLight_{ nullptr };
};

#endif
