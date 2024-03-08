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

void Component::PreInit()
{
}

void Component::Init()
{
	BeginGame();
}

void Component::PostInit()
{
	isInitialized_ = true;
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

void Component::UpdateComponentToWorldTransformationMatrix()
{
	if (parent_)
	{
		componentToWorldTransformationMatrix_ = parent_->GetComponentToWorldTransformationMatrix();

		worldPosition_ = parent_->GetWorldPosition();
		worldRotation_ = parent_->GetWorldRotation();
		worldScaling_ = parent_->GetWorldScaling();
	}
	else if(owner_)
	{
		componentToWorldTransformationMatrix_ = owner_->GetWorldTransformationMatrix();

		worldPosition_ = owner_->GetWorldPosition();
		worldRotation_ = owner_->GetWorldRotation();
		worldScaling_ = owner_->GetWorldScaling();
	}
	else
	{
		worldPosition_ = Vector3::ZeroVector;
		worldRotation_ = Quaternion::Identity;
		worldScaling_ = Vector3{ 1.f, 1.f, 1.f };
	}

	worldPosition_ += GetRelativePosition();
	worldRotation_ *= GetRelativeRotation();
	worldScaling_ *= GetRelativeScaling();

	componentToWorldTransformationMatrix_ = componentToWorldTransformationMatrix_ * GetRelativeTransformationMatrix();

	UpdateChildrenComponentToWorldTransformations();
}
