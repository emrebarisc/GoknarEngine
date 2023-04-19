#include "pch.h"

#include "Component.h"
#include "Engine.h"
#include "ObjectBase.h"

Component::Component(Component* parent) :
	parent_(parent), 
	isActive_(true),
	isTickable_(false),
	isInitialized_(false)
{
	engine->RegisterComponent(this);
}

Component::Component(ObjectBase* parentObjectBase) :
	Component(parentObjectBase->GetRootComponent())
{
}

void Component::Destroy()
{
	engine->DestroyComponent(this);
}

void Component::SetParent(ObjectBase* objectBase)
{
	parent_ = objectBase->GetRootComponent();
	if (parent_)
	{
		parent_->children_.push_back(this);
	}
}

void Component::SetParent(Component* component)
{
	parent_ = component;
	if (parent_)
	{
		parent_->children_.push_back(this);
	}
}

void Component::RemoveChild(Component* child)
{
	std::vector<Component*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		if ((*childrenIterator) == child)
		{
			children_.erase(childrenIterator);
			break;
		}
	}
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

void Component::Init()
{
	isInitialized_ = true;
	BeginGame();
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

void Component::SetRelativeRotation(const Quaternion& rotation)
{
	relativeRotation_ = rotation;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativeScaling(const Vector3& scaling)
{
	relativeScaling_ = scaling;
	UpdateRelativeTransformationMatrix();
}