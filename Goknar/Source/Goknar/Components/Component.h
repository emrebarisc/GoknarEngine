#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/ObjectBase.h"

class GOKNAR_API Component
{
public:
	Component(Component* parent);
	Component(ObjectBase* parentObjectBase);
	virtual ~Component()
	{

	}

	virtual void BeginGame() {}
	virtual void TickComponent(float deltaTime) {}

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


	bool GetIsTickable() const
	{
		return isTickable_;
	}

	void SetIsTickable(bool isTickable);

	virtual void Destroy();

	void SetParent(ObjectBase* objectBase);
	void SetParent(Component* component);
	Component* GetParent() const
	{
		return parent_;
	}

	void SetOwner(ObjectBase* owner)
	{
		owner_ = owner;
	}

	ObjectBase* GetOwner() const
	{
		return owner_;
	}

	virtual void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetIsActive() const
	{
		return isActive_;
	}

	virtual void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) {}
protected:
	Matrix relativeTransformationMatrix_{ Matrix::IdentityMatrix };
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
private:
	Vector3 pivotPoint_{ Vector3::ZeroVector };

	Vector3 relativePosition_{ Vector3::ZeroVector };
	Vector3 relativeRotation_{ Vector3::ZeroVector };
	Vector3 relativeScaling_{ Vector3(1.f) };

	ObjectBase* owner_{ nullptr };
	Component* parent_{ nullptr };

	bool isActive_{ true };
	bool isTickable_{ false };
};
#endif