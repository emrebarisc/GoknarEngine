#ifndef __SOCKETCOMPONENT_H__
#define __SOCKETCOMPONENT_H__

#include "Core.h"
#include "Component.h"

class ObjectBase;

class SocketComponent : public Component
{
public:
	SocketComponent(Component* parentComponent) :
		Component(parentComponent)
	{
	}


	virtual void Destroy() override;

	virtual const Matrix& GetRelativeTransformationMatrix() const override
	{
		return boneAndRelativeTransformationMatrix_;
	}

	void SetBoneTransformationMatrix(const Matrix& boneTransformationMatrix)
	{
		boneTransformationMatrix_ = boneTransformationMatrix;
		boneAndRelativeTransformationMatrix_ = boneTransformationMatrix_ * relativeTransformationMatrix_;
		UpdateComponentToWorldTransformationMatrix();
	}

	const Matrix& GetBoneTransformationMatrix()
	{
		return boneTransformationMatrix_;
	}

	void Attach(ObjectBase* object)
	{
		attachedObjects_.push_back(object);
	}

	void RemoveObject(ObjectBase* object)
	{
		std::vector<ObjectBase*>::iterator attachedObjectsIterator = attachedObjects_.begin();
		for (; attachedObjectsIterator != attachedObjects_.end(); ++attachedObjectsIterator)
		{
			if ((*attachedObjectsIterator) == object)
			{
				attachedObjects_.erase(attachedObjectsIterator);
				break;
			}
		}
	}

	virtual void SetIsActive(bool isActive) override;

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();
	virtual void UpdateChildrenComponentToWorldTransformations();

private:
	std::vector<ObjectBase*> attachedObjects_;

	Matrix boneTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix boneAndRelativeTransformationMatrix_{ Matrix::IdentityMatrix };
};

#endif