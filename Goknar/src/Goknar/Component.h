#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Matrix.h"

class GOKNAR_API Component
{
public:
	Component() :
		relativeTransformationMatrix_(Matrix::IdentityMatrix),
		relativeLocation_(Vector3::ZeroVector),
		relativeRotation_(Vector3::ZeroVector), 
		worldTransformationMatrix_(Matrix::IdentityMatrix),
		worldLocation_(Vector3::ZeroVector),
		worldRotation_(Vector3::ZeroVector),
		worldScaling_(Vector3(1.f)),
		componentId_(lastComponentId_++)
	{
	}

	void SetRelativeLocation(const Vector3& location);
	const Vector3& GetRelativeLocation() const
	{
		return relativeLocation_;
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

	void SetWorldLocation(const Vector3& location);
	const Vector3& GetWorldLocation() const
	{
		return worldLocation_;
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
	void UpdateRelativeTransformationMatrix();
	void UpdateWorldTransformationMatrix();

	Vector3 relativeLocation_;
	Vector3 relativeRotation_;
	Vector3 relativeScaling_;

	Vector3 worldLocation_;
	Vector3 worldRotation_;
	Vector3 worldScaling_;

	Vector3 pivotPoint_;

	int componentId_;

	static int lastComponentId_;
};

#endif