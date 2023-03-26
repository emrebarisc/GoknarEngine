#include "pch.h"

#include "Component.h"
#include "Engine.h"
#include "ObjectBase.h"

Component::Component(Component* parent) :
	parent_(parent)
{
	engine->RegisterComponent(this);
}

Component::Component(ObjectBase* parentObjectBase) :
	Component(parentObjectBase->GetRootComponent())
{
}

void Component::Destroy()
{
	delete this;
}

void Component::SetParent(ObjectBase* objectBase)
{
	parent_ = objectBase->GetRootComponent();
}

void Component::SetParent(Component* component)
{
	parent_ = component;
}

void Component::SetIsTickable(bool isTickable)
{
	isTickable_ = isTickable;
	if (isTickable_)
	{
		engine->AddToTickableComponents(this);
	}
	else
	{
		engine->RemoveFromTickableComponents(this);
	}
}

void Component::SetPivotPoint(const Vector3& pivotPoint)
{
	pivotPoint_ = pivotPoint;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativePosition(const Vector3& position)
{
	relativePosition_ = position;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativeRotation(const Vector3& rotation)
{
	relativeRotation_ = rotation;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativeScaling(const Vector3& scaling)
{
	relativeScaling_ = scaling;
	UpdateRelativeTransformationMatrix();
}