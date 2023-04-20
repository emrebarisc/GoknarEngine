#ifndef __SOCKETCOMPONENT_H__
#define __SOCKETCOMPONENT_H__

#include "Core.h"
#include "Component.h"

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

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();

private:
	Matrix boneTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix boneAndRelativeTransformationMatrix_{ Matrix::IdentityMatrix };
};

#endif