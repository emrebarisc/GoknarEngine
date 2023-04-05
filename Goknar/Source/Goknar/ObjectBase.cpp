#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Application.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

ObjectBase::ObjectBase() :
	worldTransformationMatrix_(Matrix::IdentityMatrix),
	worldPosition_(Vector3::ZeroVector),
	worldRotation_(Vector3::ZeroVector),
	worldScaling_(Vector3(1.f)),
	totalComponentCount_(0),
	isTickable_(false),
	isActive_(true),
	isInitialized_(false)
{
	engine->GetApplication()->GetMainScene()->AddObject(this);
	engine->RegisterObject(this);
}

ObjectBase::~ObjectBase()
{
	for (size_t componentIndex = 0; componentIndex < totalComponentCount_; componentIndex++)
	{
		delete components_[componentIndex];
	}
	components_.clear();
	totalComponentCount_ = 0;
}

void ObjectBase::Init()
{
	isInitialized_ = true;
	BeginGame();
}

void ObjectBase::Destroy()
{
	int componentSize = components_.size();
	for (int componentIndex = 0; componentIndex < componentSize; componentIndex++)
	{
		components_[componentIndex]->Destroy();
		components_.erase(components_.begin() + componentIndex);
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

void ObjectBase::SetWorldRotation(const Vector3& rotation)
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
	if (rootComponent_)
	{
		rootComponent_->SetParent(static_cast<Component*>(nullptr));
	}

	if (parent_)
	{
		worldPosition_ = parent_->GetWorldPosition();
		worldRotation_ = parent_->GetWorldRotation();
		worldScaling_ = parent_->GetWorldScaling();
		UpdateWorldTransformationMatrix();

		parent_ = nullptr;
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

void ObjectBase::UpdateWorldTransformationMatrix()
{
	// Since OpenGL uses column-major matriced and Goknar does not
	// all matrix multiplications are done in reverse order
	worldTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, worldPosition_.x,
										0.f, 1.f, 0.f, worldPosition_.y,
										0.f, 0.f, 1.f, worldPosition_.z,
										0.f, 0.f, 0.f, 1.f);

	Vector3 worldRotationInRadians = worldRotation_;
	worldRotationInRadians.ConvertDegreeToRadian();
	worldTransformationMatrix_ *= Matrix::GetRotationMatrix(worldRotationInRadians);

	worldTransformationMatrix_ *= Matrix(worldScaling_.x, 0.f, 0.f, 0.f,
										 0.f, worldScaling_.y, 0.f, 0.f,
										 0.f, 0.f, worldScaling_.z, 0.f,
										 0.f, 0.f, 0.f, 1.f);
}
