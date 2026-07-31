#ifndef __SPOTLIGHTCOMPONENT_H__
#define __SPOTLIGHTCOMPONENT_H__

#include "Core.h"
#include "Goknar/Components/Component.h"

class SpotLight;

class GOKNAR_API SpotLightComponent : public Component
{
public:
	SpotLightComponent(Component* parentComponent);
	virtual ~SpotLightComponent();

	virtual void Destroy() override;
	Component* Clone() const override;

	SpotLight* GetSpotLight() const
	{
		return spotLight_;
	}

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();

private:
	virtual void DestroyInner() override;

	SpotLight* spotLight_{ nullptr };
};

#endif
