#ifndef __SOCKETCOMPONENT_H__
#define __SOCKETCOMPONENT_H__

#include "Core.h"
#include "Component.h"

class SocketComponent : public Component
{
public:
	SocketComponent(Component* parentComponent);

	virtual const Matrix& GetRelativeTransformationMatrix() const override;

	void SetBoneTransformationMatrix(const Matrix& boneTransformationMatrix)
	{
		boneTransformationMatrix_ = boneTransformationMatrix;
		boneAndRelativeTransformationMatrix_ = relativeTransformationMatrix_ * boneTransformationMatrix_;
	}

	const Matrix& GetBoneTransformationMatrix()
	{
		return boneTransformationMatrix_;
	}

protected:

private:
	Matrix boneTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix boneAndRelativeTransformationMatrix_{ Matrix::IdentityMatrix };
};

#endif