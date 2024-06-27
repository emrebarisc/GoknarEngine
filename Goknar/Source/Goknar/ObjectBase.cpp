#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Managers/ObjectIDManager.h"

ObjectBase::ObjectBase() :
	totalComponentCount_(0),
	isTickable_(false),
	isTickEnabled_(true),
	isActive_(true),
	isInitialized_(false),
	isPendingDestroy_(false)
{
	engine->RegisterObject(this);

	id_ = ObjectIDManager::GetInstance()->GetAndIncreaseObjectBaseID();
}

ObjectBase::~ObjectBase()
{
}

void ObjectBase::PreInit()
{
	if (isInitialized_)
	{
		GOKNAR_CORE_ASSERT("ObjectBase::Init called more than once!");
		return;
	}
}

void ObjectBase::Init()
{
}

void ObjectBase::PostInit()
{
	isInitialized_ = true;
}

void ObjectBase::Destroy()
{
	if(isPendingDestroy_)
	{
		return;
	}
	
	isPendingDestroy_ = true;
	engine->AddObjectToDestroy(this);

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
}

void ObjectBase::DestroyInner()
{
	// Detach from the socket component if any
	if (parentSocket_)
	{
		parentSocket_->RemoveObject(this);
	}
}

void ObjectBase::SetRootComponent(Component* newRootComponent)
{
	if(newRootComponent->parent_)
	{
		newRootComponent->parent_->RemoveChild(newRootComponent);
		newRootComponent->parent_ = nullptr;
	}

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

void ObjectBase::SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix/* = true*/)
{
	worldPosition_ = position;
	if (updateWorldTransformationMatrix)
	{
		UpdateWorldTransformationMatrix();
	}
}

void ObjectBase::SetWorldRotation(const Quaternion& rotation, bool updateWorldTransformationMatrix/* = true*/)
{
	worldRotation_ = rotation;
	if (updateWorldTransformationMatrix)
	{
		UpdateWorldTransformationMatrix();
	}
}

void ObjectBase::SetWorldScaling(const Vector3& scaling, bool updateWorldTransformationMatrix/* = true*/)
{
	worldScaling_ = scaling;
	if (updateWorldTransformationMatrix)
	{
		UpdateWorldTransformationMatrix();
	}
}

void ObjectBase::SetIsActive(bool isActive)
{
	isActive_ = isActive;

	for (int i = 0; i < children_.size(); i++)
	{
		children_[i]->SetIsActive(isActive);
	}

	for (int i = 0; i < components_.size(); i++)
	{
		components_[i]->SetIsActive(isActive);
	}
}

void ObjectBase::AttachToSocket(SocketComponent* socketComponent)
{
	SetParent(nullptr);

	parentSocket_ = socketComponent;
	socketComponent->Attach(this);
}

void ObjectBase::RemoveFromSocket(SocketComponent* socketComponent)
{
	parentSocket_ = nullptr;
	socketComponent->RemoveObject(this);
}

void ObjectBase::SetParent(ObjectBase* newParent, SnappingRule snappingRule/* = SnappingRule::KeepWorldAll*/, bool updateWorldTransformation/* = true*/)
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
			// In case of getting another ObjectBase as a parent
			// World transformations(Position, rotation and scaling)
			// Act like relative transformations
			// Just like in Component's relative transformations

			if((unsigned char)snappingRule & (unsigned char)SnappingRule::KeepWorldPosition)
			{
				worldPosition_ -= newParent->GetWorldPosition();
			}
			else
			{
				worldPosition_ = Vector3::ZeroVector;
			}
			
			if((unsigned char)snappingRule & (unsigned char)SnappingRule::KeepWorldRotation)
			{
				//Vector3 eulerThis = worldRotation_.ToEulerRadians();
				//Vector3 eulerParent = newParent->GetWorldRotation().ToEulerRadians();
				//worldRotation_ = Quaternion::FromEulerRadians(eulerThis - eulerParent);

				worldRotation_ = newParent->GetWorldRotation().GetInverse() * worldRotation_;
			}
			else
			{
				worldRotation_ = Quaternion::Identity;
			}
			
			if((unsigned char)snappingRule & (unsigned char)SnappingRule::KeepWorldScaling)
			{
				worldScaling_ = worldScaling_ / newParent->GetWorldScaling();
			}
			else
			{
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

Vector3 ObjectBase::GetRelativePositionInWorldSpace(const Vector3& relativePosition)
{
	return worldTransformationMatrix_ * Vector4(relativePosition, 1.f);
}

Vector3 ObjectBase::GetWorldPositionInRelativeSpace(const Vector3& positionInWorldSpace)
{
	return worldTransformationMatrix_.GetInverse() * Vector4(positionInWorldSpace, 1.f);
}

Vector3 ObjectBase::GetRelativeDirectionInWorldSpace(const Vector3& relativeDirection)
{
	return worldTransformationMatrix_ * Vector4(relativeDirection, 0.f);
}

Vector3 ObjectBase::GetWorldDirectionInRelativeSpace(const Vector3& directionInWorldSpace)
{
	return worldTransformationMatrix_.GetInverse() * Vector4(directionInWorldSpace, 0.f);
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
	if (worldTransformationMatrix.ContainsNanOrInf())
	{
		GOKNAR_FATAL("NAN OR INF VALUE ON TRANSFORMATION MATRIX");
	}

	worldTransformationMatrix_ = worldTransformationMatrix;
	UpdateChildrenTransformations();
}

void ObjectBase::UpdateWorldTransformationMatrix()
{
	if (parent_)
	{
		Matrix thisObjectWorldTransformationMatrixWithoutScaling = Matrix::GetPositionMatrix(worldPosition_) * worldRotation_.GetMatrix();

		worldTransformationMatrixWithoutScaling_ = parent_->worldTransformationMatrixWithoutScaling_;
		worldTransformationMatrixWithoutScaling_ *= thisObjectWorldTransformationMatrixWithoutScaling;

		worldTransformationMatrix_ = parent_->GetWorldTransformationMatrix();
		worldTransformationMatrix_ *= thisObjectWorldTransformationMatrixWithoutScaling * Matrix::GetScalingMatrix(worldScaling_);
	}
	else
	{
		worldTransformationMatrixWithoutScaling_ = Matrix::GetPositionMatrix(worldPosition_) * worldRotation_.GetMatrix();

		worldTransformationMatrix_ = worldTransformationMatrixWithoutScaling_ * Matrix::GetScalingMatrix(worldScaling_);
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
		rootComponent_->UpdateComponentToWorldTransformationMatrix();
	}
}
