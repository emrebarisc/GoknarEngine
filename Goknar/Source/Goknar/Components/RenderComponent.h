#ifndef __RENDERCOMPONENT_H__
#define __RENDERCOMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Math/Matrix.h"

class ObjectBase;

class GOKNAR_API RenderComponent : public Component
{
public:
	RenderComponent() = delete;
	RenderComponent(ObjectBase* parent);
	virtual ~RenderComponent()
	{

	}

	virtual void Destroy() = 0;

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
	
	virtual void SetIsRendered(bool isRendered) = 0;
	
	bool GetIsRendered() const
	{
		return isRendered_;
	}

	virtual void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) = 0;

protected:
	Matrix relativeTransformationMatrix_;
	inline virtual void UpdateRelativeTransformationMatrix()
	{
		// Since OpenGL uses column-major matriced and Goknar does not
		// all matrix multiplications are done in reverse order
		
		relativeTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, pivotPoint_.x,
											0.f, 1.f, 0.f, pivotPoint_.y,
											0.f, 0.f, 1.f, pivotPoint_.z,
											0.f, 0.f, 0.f, 1.f);

		relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, relativePosition_.x,
												0.f, 1.f, 0.f, relativePosition_.y,
												0.f, 0.f, 1.f, relativePosition_.z,
												0.f, 0.f, 0.f, 1.f);

		Vector3 relativeRotationInRadians = relativeRotation_;
		relativeRotationInRadians.ConvertDegreeToRadian();
		relativeTransformationMatrix_ *= Matrix::GetRotationMatrix(relativeRotationInRadians);

		relativeTransformationMatrix_ *= Matrix(relativeScaling_.x, 0.f, 0.f, 0.f,
												0.f, relativeScaling_.y, 0.f, 0.f,
												0.f, 0.f, relativeScaling_.z, 0.f,
												0.f, 0.f, 0.f, 1.f);

		relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, -pivotPoint_.x,
												0.f, 1.f, 0.f, -pivotPoint_.y,
												0.f, 0.f, 1.f, -pivotPoint_.z,
												0.f, 0.f, 0.f, 1.f);
	}

	bool isRendered_;
private:
	Vector3 pivotPoint_;

	Vector3 relativePosition_;
	Vector3 relativeRotation_;
	Vector3 relativeScaling_;

	ObjectBase* parent_;
};
#endif