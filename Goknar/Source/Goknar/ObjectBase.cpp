#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Application.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"

ObjectBase::ObjectBase() :
	totalComponentCount_(0),
	isTickable_(false),
	isActive_(true),
	isInitialized_(false)
{
	engine->RegisterObject(this);
}

ObjectBase::~ObjectBase()
{
}

void ObjectBase::Init()
{
	if (isInitialized_)
	{
		GOKNAR_CORE_ERROR("ObjectBase::Init called more than once!");
		return;
	}

	engine->GetApplication()->GetMainScene()->AddObject(this);
	BeginGame();
	isInitialized_ = true;
}

void ObjectBase::Destroy()
{
	if (rootComponent_)
	{
		// Detach from the socket component if any
		SocketComponent* attachedSocketComponent = dynamic_cast<SocketComponent*>(rootComponent_->GetParent());
		if (attachedSocketComponent)
		{
			attachedSocketComponent->RemoveChild(rootComponent_);
		}
	}

	std::vector<ObjectBase*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		(*childrenIterator)->Destroy();
	}

	int componentSize = components_.size();
	for (int componentIndex = 0; componentIndex < componentSize; componentIndex++)
	{
		components_[componentIndex]->Destroy();
	}
	engine->DestroyObject(this);
	engine->GetApplication()->GetMainScene()->RemoveObject(this);
}

void ObjectBase::SetRootComponent(Component* newRootComponent)
{
	std::vector<Component *>::iterator componentIterator = components_.begin();
	for (; componentIterator != components_.end(); ++componentIterator)
	{
		Component* component = *componentIterator;
		if (component->GetParent() == rootComponent_)
		{
			component->SetParent(newRootComponent);
		}
	}

	rootComponent_ = newRootComponent;
}

void ObjectBase::SetIsTickable(bool isTickable)
{
	isTickable_ = isTickable;
	if (isTickable_)
	{
		engine->AddToTickableObjects(this);
	}
	else
	{
		engine->RemoveFromTickableObjects(this);
	}
}

void ObjectBase::SetWorldPosition(const Vector3& position)
{
	worldPosition_ = position;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::SetWorldRotation(const Quaternion& rotation)
{
	worldRotation_ = rotation;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::SetWorldScaling(const Vector3& scaling)
{
	worldScaling_ = scaling;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::SetIsActive(bool isActive)
{
	isActive_ = isActive;
	for (int i = 0; i < components_.size(); i++)
	{
		components_[i]->SetIsActive(isActive);
	}
}

void ObjectBase::AttachToSocket(SocketComponent* socketComponent)
{
	SetParent(nullptr);

	if (rootComponent_)
	{
		rootComponent_->SetParent(socketComponent);
	}
}

void ObjectBase::RemoveFromSocket(SocketComponent* socketComponent)
{
	if (rootComponent_)
	{
		SocketComponent* rootSocket = dynamic_cast<SocketComponent*>(rootComponent_->GetParent());
		if (!rootSocket || rootSocket != socketComponent)
		{
			return;
		}

		socketComponent->RemoveChild(rootComponent_);
		rootComponent_->SetParent(static_cast<Component*>(nullptr));

		worldPosition_ = rootComponent_->GetWorldPosition();
		worldRotation_ = rootComponent_->GetWorldRotation();
		worldScaling_ = rootComponent_->GetWorldScaling();
		
		worldTransformationMatrix_ = rootComponent_->GetComponentToWorldTransformationMatrix();
		UpdateChildrenTransformations();
	}
}

void ObjectBase::SetParent(ObjectBase* newParent, bool resetTransformation/* = false*/, bool updateWorldTransformation/* = true*/)
{
	if (parent_)
	{
		parent_->RemoveChild(this);
	}

	parent_ = newParent;

	if (newParent)
	{
		newParent->AddChild(this);

		if (updateWorldTransformation)
		{
			if (resetTransformation)
			{
				worldPosition_ = Vector3::ZeroVector;
				worldRotation_ = Quaternion::Identity;
				worldScaling_ = Vector3{ 1.f };
			}

			UpdateWorldTransformationMatrix();
		}
	}
}

void ObjectBase::RemoveChild(ObjectBase* child)
{
	std::vector<ObjectBase*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		if ((*childrenIterator) == child)
		{
			children_.erase(childrenIterator);
			return;
		}
	}
}

void ObjectBase::AddComponent(Component* component)
{
	if (totalComponentCount_ == 0)
	{
		rootComponent_ = component;
	}
	else
	{
		component->SetParent(rootComponent_);
	}

	component->SetOwner(this);
	components_.push_back(component);
	totalComponentCount_++;
}

void ObjectBase::SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix)
{
	worldTransformationMatrix_ = worldTransformationMatrix;
	UpdateChildrenTransformations();
}

void ObjectBase::UpdateWorldTransformationMatrix()
{
	if (parent_)
	{
		worldTransformationMatrix_ = parent_->GetWorldTransformationMatrix();
		worldTransformationMatrix_ *= Matrix::GetTransformationMatrix(worldRotation_, worldPosition_, worldScaling_);
	}
	else
	{
		worldTransformationMatrix_ = Matrix::GetTransformationMatrix(worldRotation_, worldPosition_, worldScaling_);
	}

	UpdateChildrenTransformations();
}

void ObjectBase::UpdateChildrenTransformations()
{
	std::vector<ObjectBase*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		ObjectBase* child = *childrenIterator;
		child->UpdateWorldTransformationMatrix();
	}

	if (rootComponent_)
	{
		Component* socketParent = rootComponent_->GetParent();
		if (socketParent)
		{
			socketParent->UpdateComponentToWorldTransformationMatrix();
		}
		else
		{
			rootComponent_->UpdateComponentToWorldTransformationMatrix();
		}
	}
}
