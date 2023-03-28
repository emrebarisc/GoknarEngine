#include "SocketComponent.h"

SocketComponent::SocketComponent(Component* parentComponent) :
	Component(parentComponent)
{
}

const Matrix& SocketComponent::GetRelativeTransformationMatrix() const
{
	return boneAndRelativeTransformationMatrix_;
}
