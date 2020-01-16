#ifndef __MODEL_H__
#define __MODEL_H__

#include "Goknar/Matrix.h"

class GOKNAR_API Model
{
public:
	Model() :
		relativeTransformationMatrix_(Matrix::IdentityMatrix),
		relativePosition_(Vector3::ZeroVector),
		relativeRotation_(Vector3::ZeroVector),
		relativeScaling_(Vector3(1.f)),
		worldTransformationMatrix_(Matrix::IdentityMatrix),
		worldPosition_(Vector3::ZeroVector),
		worldRotation_(Vector3::ZeroVector),
		worldScaling_(Vector3(1.f)),
		componentId_(lastComponentId_++)
	{
	}

	void SetRelativePosition(const Vector3& position);
	const Vector3& GetRelativePosition() const
	{
		return relativePosition_;
	}

	void SetRelativeRotation(const Vector3& rotation);
	const Vector3& GetRelativeRotation() const
	{
		return relativeRotation_;
	}

	void SetRelativeScaling(const Vector3& scaling);
	const Vector3& GetRelativeScaling() const
	{
		return relativeScaling_;
	}
	
	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void SetWorldPosition(const Vector3& position);
	const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	void SetWorldRotation(const Vector3& rotation);
	const Vector3& GetWorldRotation() const
	{
		return worldRotation_;
	}

	void SetWorldScaling(const Vector3& scaling);
	const Vector3& GetWorldScaling() const
	{
		return worldScaling_;
	}

	const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	void SetPivotPoint(const Vector3& pivotPoint)
	{
		pivotPoint_ = pivotPoint;
	}

	const Vector3& GetPivotPoint() const
	{
		return pivotPoint_;
	}

	int GetComponentId() const
	{
		return componentId_;
	}

	void SetComponentId(int componentId)
	{
		componentId_ = componentId;
	}

protected:
	Matrix relativeTransformationMatrix_;
	Matrix worldTransformationMatrix_;

private:
	inline void UpdateRelativeTransformationMatrix();
	inline void UpdateWorldTransformationMatrix();

	Vector3 pivotPoint_;

	Vector3 relativePosition_;
	Vector3 relativeRotation_;
	Vector3 relativeScaling_;

	Vector3 worldPosition_;
	Vector3 worldRotation_;
	Vector3 worldScaling_;

	int componentId_;

	static int lastComponentId_;
};

#endif