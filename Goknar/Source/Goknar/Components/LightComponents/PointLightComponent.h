#ifndef __POINTLIGHTCOMPONENT_H__
#define __POINTLIGHTCOMPONENT_H__

#include "Core.h"
#include "Goknar/Components/Component.h"

class PointLight;

class GOKNAR_API PointLightComponent : public Component
{
public:
	PointLightComponent(Component* parentComponent);
	virtual ~PointLightComponent();

	virtual void Destroy() override;
	Component* Clone() const override;

	PointLight* GetPointLight() const
	{
		return pointLight_;
	}

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();

private:
	virtual void DestroyInner() override;

	PointLight* pointLight_{ nullptr };
};

#endif
