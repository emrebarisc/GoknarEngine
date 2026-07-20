#include "pch.h"

#include "ObjectBase.h"

#include <unordered_map>

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Log.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Managers/ObjectIDManager.h"

ObjectBase::ObjectBase(const ObjectInitializer& objectInitializer) :
	totalComponentCount_(0),
	isTickable_(false),
	isTickEnabled_(true),
	isActive_(true),
	isInitialized_(false),
	isPendingDestroy_(false)
{
	engine->RegisterObject(this);

	if (objectInitializer.setForInitializeOnConstructor)
	{
		engine->AddToObjectsToBeInitialized(this);
	}

	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseObjectBaseGUID();
}

ObjectBase::~ObjectBase()
{
}

ObjectBase* ObjectBase::Clone() const
{
	ObjectBase* clonedObject = new ObjectBase();
	CopyValuesTo(clonedObject);

	return clonedObject;
}

void ObjectBase::PreInit()
{
	if (isInitialized_)
	{
		GOKNAR_CORE_ASSERT(false, "ObjectBase::Init called more than once!");
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

void ObjectBase::BeginGame()
{
	UpdateWorldTransformationMatrix();
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
		ObjectBase* child = *childrenIterator;
		child->Destroy();
		child->parent_ = nullptr;
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

	if (parent_)
	{
		parent_->RemoveChild(this);
	}
}

void ObjectBase::CopyValuesTo(ObjectBase* object) const
{
	if (!object)
	{
		return;
	}

	object->SetName(name_);
	object->SetWorldPosition(worldPosition_, false);
	object->SetWorldRotation(worldRotation_, false);
	object->SetWorldScaling(worldScaling_, false);
	object->UpdateWorldTransformationMatrix();
	if (object->isTickable_ != isTickable_)
	{
		object->SetIsTickable(isTickable_);
	}
	object->SetIsTickEnabled(isTickEnabled_);
	object->isActive_ = isActive_;

	std::unordered_map<const Component*, Component*> clonedComponents;

	const auto cloneComponent =
		[object, &clonedComponents](const Component* component)
		{
			if (!component || clonedComponents.find(component) != clonedComponents.end())
			{
				return;
			}

			Component* clonedComponent = component->Clone();
			if (!clonedComponent)
			{
				return;
			}

			object->AddComponent(clonedComponent);
			clonedComponents[component] = clonedComponent;
		};

	cloneComponent(rootComponent_);

	for (Component* component : components_)
	{
		cloneComponent(component);
	}

	for (const auto& clonedComponentPair : clonedComponents)
	{
		const Component* sourceComponent = clonedComponentPair.first;
		Component* clonedComponent = clonedComponentPair.second;
		Component* sourceParent = sourceComponent->GetParent();
		auto clonedParentIterator = clonedComponents.find(sourceParent);

		if (clonedParentIterator != clonedComponents.end())
		{
			clonedComponent->SetParent(clonedParentIterator->second);
		}
		else
		{
			clonedComponent->SetParent((Component*)nullptr);
		}
	}

	auto clonedRootComponentIterator = clonedComponents.find(rootComponent_);
	if (clonedRootComponentIterator != clonedComponents.end())
	{
		object->SetRootComponent(clonedRootComponentIterator->second);
	}

	for (ObjectBase* child : children_)
	{
		if (!child)
		{
			continue;
		}

		ObjectBase* clonedChild = child->Clone();
		if (clonedChild)
		{
			clonedChild->SetParent(object, SnappingRule::KeepWorldAll, false);
		}
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
	worldRotation_ = rotation.GetNormalized();
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
	if (newParent == this)
	{
		GOKNAR_CORE_ASSERT(false, "ObjectBase cannot be parented to itself.");
		return;
	}

	for (ObjectBase* ancestor = newParent; ancestor; ancestor = ancestor->parent_)
	{
		if (ancestor == this)
		{
			GOKNAR_CORE_ASSERT(false, "ObjectBase cannot be parented to one of its descendants.");
			return;
		}
	}

	if (newParent == parent_)
	{
		if (updateWorldTransformation)
		{
			UpdateWorldTransformationMatrix();
		}
		return;
	}

	const auto hasSnappingRule =
		[](SnappingRule snappingRule, SnappingRule flag)
		{
			return (static_cast<unsigned char>(snappingRule) & static_cast<unsigned char>(flag)) != 0;
		};
	const auto hasZeroScaleComponent =
		[](const Vector3& scaling)
		{
			return GoknarMath::Abs(scaling.x) <= SMALLER_EPSILON ||
				GoknarMath::Abs(scaling.y) <= SMALLER_EPSILON ||
				GoknarMath::Abs(scaling.z) <= SMALLER_EPSILON;
		};
	const auto getActualWorldRotation =
		[](const ObjectBase* object)
		{
			if (!object)
			{
				return Quaternion::Identity;
			}

			Quaternion rotation = object->GetWorldRotation();
			for (ObjectBase* parent = object->GetParent(); parent; parent = parent->GetParent())
			{
				rotation = parent->GetWorldRotation() * rotation;
			}

			return rotation.GetNormalized();
		};
	const auto getActualWorldScaling =
		[](const ObjectBase* object)
		{
			Vector3 scaling(1.f);
			for (const ObjectBase* current = object; current; current = current->GetParent())
			{
				scaling *= current->GetWorldScaling();
			}

			return scaling;
		};

	const bool keepWorldPosition = hasSnappingRule(snappingRule, SnappingRule::KeepWorldPosition);
	const bool keepWorldRotation = hasSnappingRule(snappingRule, SnappingRule::KeepWorldRotation);
	const bool keepWorldScaling = hasSnappingRule(snappingRule, SnappingRule::KeepWorldScaling);

	Vector3 previousWorldPosition = Vector3::ZeroVector;
	Quaternion previousWorldRotation = Quaternion::Identity;
	Vector3 previousWorldScaling(1.f);
	Vector3 newParentWorldScaling(1.f);

	if (updateWorldTransformation)
	{
		previousWorldPosition = GetWorldTransformationMatrix().GetTranslation();
		previousWorldRotation = getActualWorldRotation(this);
		previousWorldScaling = getActualWorldScaling(this);

		if (newParent)
		{
			newParentWorldScaling = getActualWorldScaling(newParent);

			if ((keepWorldPosition || keepWorldScaling) && hasZeroScaleComponent(newParentWorldScaling))
			{
				GOKNAR_CORE_ASSERT(false, "Cannot keep world transform while parent has zero scale.");
				return;
			}
		}
	}

	if (parent_)
	{
		parent_->RemoveChild(this);
	}

	if (parentSocket_)
	{
		parentSocket_->RemoveObject(this);
		parentSocket_ = nullptr;
	}

	parent_ = newParent;

	if (newParent)
	{
		newParent->AddChild(this);
	}

	if (!updateWorldTransformation)
	{
		return;
	}

	if (newParent)
	{
		worldPosition_ = keepWorldPosition ? newParent->GetWorldPositionInRelativeSpace(previousWorldPosition) : Vector3::ZeroVector;
		worldRotation_ = keepWorldRotation ? (getActualWorldRotation(newParent).GetInverse() * previousWorldRotation).GetNormalized() : Quaternion::Identity;
		worldScaling_ = keepWorldScaling ? previousWorldScaling / newParentWorldScaling : Vector3(1.f);
	}
	else
	{
		if (keepWorldPosition)
		{
			worldPosition_ = previousWorldPosition;
		}
		if (keepWorldRotation)
		{
			worldRotation_ = previousWorldRotation;
		}
		if (keepWorldScaling)
		{
			worldScaling_ = previousWorldScaling;
		}
	}

	UpdateWorldTransformationMatrix();
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

	const auto hasZeroScaleComponent =
		[](const Vector3& scaling)
		{
			return GoknarMath::Abs(scaling.x) <= SMALLER_EPSILON ||
				GoknarMath::Abs(scaling.y) <= SMALLER_EPSILON ||
				GoknarMath::Abs(scaling.z) <= SMALLER_EPSILON;
		};
	const auto getActualWorldScaling =
		[](const ObjectBase* object)
		{
			Vector3 scaling(1.f);
			for (const ObjectBase* current = object; current; current = current->GetParent())
			{
				scaling *= current->GetWorldScaling();
			}

			return scaling;
		};

	if (parent_ && hasZeroScaleComponent(getActualWorldScaling(parent_)))
	{
		GOKNAR_CORE_ASSERT(false, "Cannot set world transformation matrix while parent has zero scale.");
		return;
	}

	const Matrix relativeTransformationMatrix = parent_ ?
		parent_->GetWorldTransformationMatrix().GetInverse() * worldTransformationMatrix :
		worldTransformationMatrix;

	relativeTransformationMatrix.Decompose(worldPosition_, worldScaling_, worldRotation_);
	worldRotation_.Normalize();
	UpdateWorldTransformationMatrix();
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
