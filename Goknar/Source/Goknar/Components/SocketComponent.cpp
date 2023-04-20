#include "SocketComponent.h"

void SocketComponent::Destroy()
{
	std::vector<Component*>::iterator childrenIterator = children_.begin();
	for (; childrenIterator != children_.end(); ++childrenIterator)
	{
		Component* child = *childrenIterator;

		child->GetOwner()->RemoveFromSocket(this);
	}

	Component::Destroy();
}

void SocketComponent::UpdateComponentToWorldTransformationMatrix()
{
	if (parent_)
	{
		componentToWorldTransformationMatrix_ = parent_->GetComponentToWorldTransformationMatrix();
	}
	else
	{
		componentToWorldTransformationMatrix_ = owner_->GetWorldTransformationMatrix();
	}

	componentToWorldTransformationMatrix_ = componentToWorldTransformationMatrix_ * GetRelativeTransformationMatrix();

	worldPosition_ = Vector3(componentToWorldTransformationMatrix_[3], componentToWorldTransformationMatrix_[7], componentToWorldTransformationMatrix_[11]);

	Vector3 x = Vector3(componentToWorldTransformationMatrix_[0], componentToWorldTransformationMatrix_[4], componentToWorldTransformationMatrix_[8]);
	Vector3 y = Vector3(componentToWorldTransformationMatrix_[1], componentToWorldTransformationMatrix_[5], componentToWorldTransformationMatrix_[9]);
	Vector3 z = Vector3(componentToWorldTransformationMatrix_[2], componentToWorldTransformationMatrix_[6], componentToWorldTransformationMatrix_[10]);
	worldScaling_ = Vector3(x.Length(), y.Length(), z.Length());

	Matrix3x3 unscaledComponentToWorldTransformationMatrix3x3(
		componentToWorldTransformationMatrix_[0] / worldScaling_.x, componentToWorldTransformationMatrix_[1] / worldScaling_.y, componentToWorldTransformationMatrix_[2] / worldScaling_.z,
		componentToWorldTransformationMatrix_[4] / worldScaling_.x, componentToWorldTransformationMatrix_[5] / worldScaling_.y, componentToWorldTransformationMatrix_[6] / worldScaling_.z,
		componentToWorldTransformationMatrix_[8] / worldScaling_.x, componentToWorldTransformationMatrix_[9] / worldScaling_.y, componentToWorldTransformationMatrix_[10] / worldScaling_.z
	);

	worldRotation_ = Quaternion(unscaledComponentToWorldTransformationMatrix3x3);

	UpdateChildrenComponentToWorldTransformations();
}
