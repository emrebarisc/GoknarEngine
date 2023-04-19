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
	// Detach from the socket component if any
	SocketComponent* attachedSocketComponent = dynamic_cast<SocketComponent*>(rootComponent_->GetParent());
	if (attachedSocketComponent)
	{
		attachedSocketComponent->RemoveChild(rootComponent_);
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
	if (rootComponent_)
	{
		rootComponent_->SetParent(socketComponent);
	}
	parent_ = socketComponent->GetOwner();
}

void ObjectBase::RemoveFromSocket(SocketComponent* socketComponent)
{
	SocketComponent* rootSocket = dynamic_cast<SocketComponent*>(rootComponent_->GetParent());
	if (!rootSocket || rootSocket != socketComponent)
	{
		return;
	}

	if (rootComponent_)
	{
		socketComponent->RemoveChild(rootComponent_);
		rootComponent_->SetParent(static_cast<Component*>(nullptr));
	}

	worldPosition_ = socketComponent->GetWorldPosition();
	worldRotation_ = socketComponent->GetWorldRotation();
	worldScaling_ = socketComponent->GetWorldScaling();
	UpdateWorldTransformationMatrix();

	parent_ = nullptr;
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

void ObjectBase::UpdateWorldTransformationMatrix()
{
	worldTransformationMatrix_ = Matrix::GetTransformationMatrix(worldRotation_, worldPosition_, worldScaling_);

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
