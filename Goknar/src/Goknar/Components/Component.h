#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Matrix.h"

class ObjectBase;

class GOKNAR_API Component
{
public:
	Component() = delete;
	Component(ObjectBase* parent);
	~Component()
	{

	}

	const ObjectBase* GetParent() const
	{
		return parent_;
	}

	void SetPivotPoint(const Vector3& pivotPoint);
	const Vector3& GetPivotPoint() const
	{
		return pivotPoint_;
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
	
	Vector3 GetForwardVector()
	{
		return Vector3(relativeTransformationMatrix_[0], relativeTransformationMatrix_[4], relativeTransformationMatrix_[8]);
	}

	Vector3 GetLeftVector()
	{
		return Vector3(relativeTransformationMatrix_[1], relativeTransformationMatrix_[5], relativeTransformationMatrix_[9]);
	}

	Vector3 GetUpVector()
	{
		return Vector3(relativeTransformationMatrix_[2], relativeTransformationMatrix_[6], relativeTransformationMatrix_[10]);
	}

	virtual void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) = 0;

protected:
	Matrix relativeTransformationMatrix_;
	inline virtual void UpdateRelativeTransformationMatrix();

private:
	Vector3 pivotPoint_;

	Vector3 relativePosition_;
	Vector3 relativeRotation_;
	Vector3 relativeScaling_;

	ObjectBase* parent_;

};
#endif