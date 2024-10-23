#include "pch.h"

#include "Component.h"
#include "Engine.h"
#include "ObjectBase.h"
#include "Managers/ObjectIDManager.h"

Component::Component(Component* parent) :
	parent_(parent), 
	isActive_(true),
	isTickable_(false),
	isTickEnabled_(true),
	isInitialized_(false),
	isPendingDestroy_(false)
{
	engine->RegisterComponent(this);
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseComponentID();
}

Component::Component(ObjectBase* parentObjectBase) :
	Component(parentObjectBase->GetRootComponent())
{
}

void Component::Destroy()
{
	if(isPendingDestroy_)
	{
		return;
	}

	isPendingDestroy_ = true;
	engine->AddComponentToDestroy(this);
	
	std::vector<Component*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		(*childrenIterator)->Destroy();
	}
}

void Component::DestroyInner()
{
}

void Component::SetParent(ObjectBase* objectBase)
{
	SetParent(objectBase->GetRootComponent());
}

void Component::SetParent(Component* component)
{
	if(parent_)
	{
		parent_->RemoveChild(this);
	}

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

void Component::SetRelativePosition(const Vector3& position, bool updateRelativeTransformationMatrix/* = true*/)
{
	relativePosition_ = position;
	if (updateRelativeTransformationMatrix)
	{
		UpdateRelativeTransformationMatrix();
	}
}

void Component::SetRelativeRotation(const Quaternion& rotation, bool updateRelativeTransformationMatrix/* = true*/)
{
	relativeRotation_ = rotation;
	if (updateRelativeTransformationMatrix)
	{
		UpdateRelativeTransformationMatrix();
	}
}

void Component::SetRelativeScaling(const Vector3& scaling, bool updateRelativeTransformationMatrix/* = true*/)
{
	relativeScaling_ = scaling;
	if (updateRelativeTransformationMatrix)
	{
		UpdateRelativeTransformationMatrix();
	}
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
	else if (owner_)
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

	Vector3 relativePosition = relativePosition_ * worldScaling_;
	relativePosition = relativePosition.RotatePoint(worldRotation_);
	worldPosition_ += relativePosition;

	worldRotation_ *= relativeRotation_;
	worldScaling_ *= relativeScaling_;

	componentToWorldTransformationMatrix_ = componentToWorldTransformationMatrix_ * GetRelativeTransformationMatrix();

	UpdateChildrenComponentToWorldTransformations();
}