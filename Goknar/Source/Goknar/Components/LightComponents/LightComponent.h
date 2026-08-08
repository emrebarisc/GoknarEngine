#ifndef __LIGHTCOMPONENT_H__
#define __LIGHTCOMPONENT_H__

#include "Core.h"
#include "Goknar/Components/Component.h"

template<class T>
class GOKNAR_API LightComponent : public Component
{
public:
	LightComponent(Component* parentComponent);
	virtual ~LightComponent();

	T* GetLight() const
	{
		return light_;
	}

	void SetIsActive(bool isActive) override;

protected:
	T* light_{ nullptr };

private:
	virtual void DestroyInner() override;
};

template<class T>
LightComponent<T>::LightComponent(Component* parentComponent) :
	Component(parentComponent),
	light_(new T())
{
}

template<class T>
LightComponent<T>::~LightComponent()
{
}

template<class T>
void LightComponent<T>::DestroyInner()
{
	Component::DestroyInner();
	delete light_;
}

template<class T>
void LightComponent<T>::SetIsActive(bool isActive)
{
	Component::SetIsActive(isActive);

	light_->SetIsActive(isActive);
}

#endif
