#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/ObjectBase.h"

class GOKNAR_API Component
{
	friend ObjectBase;

public:
	virtual ~Component()
	{

	}

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();
	virtual void BeginGame() {}
	virtual void TickComponent(float deltaTime) {}

	void SetPivotPoint(const Vector3& pivotPoint);
	const Vector3& GetPivotPoint() const
	{
		return pivotPoint_;
	}

	void SetRelativePosition(const Vector3& position);
	virtual const Vector3& GetRelativePosition() const
	{
		return relativePosition_;
	}

	void SetRelativeRotation(const Quaternion& rotation);
	virtual const Quaternion& GetRelativeRotation() const
	{
		return relativeRotation_;
	}

	void SetRelativeScaling(const Vector3& scaling);
	virtual const Vector3& GetRelativeScaling() const
	{
		return relativeScaling_;
	}

	virtual const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	virtual const Quaternion& GetWorldRotation() const
	{
		return worldRotation_;
	}

	virtual const Vector3& GetWorldScaling() const
	{
		return worldScaling_;
	}

	virtual const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	Vector3 GetForwardVector()
	{
		const Matrix& relativeTransformationMatrix = GetRelativeTransformationMatrix();
		return Vector3(relativeTransformationMatrix[0], relativeTransformationMatrix[4], relativeTransformationMatrix[8]).GetNormalized();
	}

	Vector3 GetLeftVector()
	{
		const Matrix& relativeTransformationMatrix = GetRelativeTransformationMatrix();
		return Vector3(relativeTransformationMatrix[1], relativeTransformationMatrix[5], relativeTransformationMatrix[9]).GetNormalized();
	}

	Vector3 GetUpVector()
	{
		const Matrix& relativeTransformationMatrix = GetRelativeTransformationMatrix();
		return Vector3(relativeTransformationMatrix[2], relativeTransformationMatrix[6], relativeTransformationMatrix[10]).GetNormalized();
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
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

	void RemoveChild(Component* child);
	const std::vector<Component*>& GetChildren()
	{
		return children_;
	}

	void SetOwner(ObjectBase* owner)
	{
		owner_ = owner;
		UpdateRelativeTransformationMatrix();
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

	const Matrix& GetComponentToWorldTransformationMatrix() const
	{
		return componentToWorldTransformationMatrix_;
	}

protected:
	Component(Component* parent);
	Component(ObjectBase* parentObjectBase);

	inline virtual void UpdateRelativeTransformationMatrix()
	{
		// Since OpenGL uses column-major matriced and Goknar does not
		// all matrix multiplications are done in reverse order

		relativeTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, pivotPoint_.x,
			0.f, 1.f, 0.f, pivotPoint_.y,
			0.f, 0.f, 1.f, pivotPoint_.z,
			0.f, 0.f, 0.f, 1.f);

		relativeTransformationMatrix_ *= Matrix::GetTransformationMatrix(relativeRotation_, relativePosition_, relativeScaling_);

		relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, -pivotPoint_.x,
			0.f, 1.f, 0.f, -pivotPoint_.y,
			0.f, 0.f, 1.f, -pivotPoint_.z,
			0.f, 0.f, 0.f, 1.f);

		UpdateComponentToWorldTransformationMatrix();
	}

	inline virtual void UpdateComponentToWorldTransformationMatrix()
	{
		if (parent_)
		{
			componentToWorldTransformationMatrix_ = parent_->GetComponentToWorldTransformationMatrix();

			worldPosition_ = parent_->GetWorldPosition();
			worldRotation_ = parent_->GetWorldRotation();
			worldScaling_ = parent_->GetWorldScaling();
		}
		else
		{
			componentToWorldTransformationMatrix_ = owner_->GetWorldTransformationMatrix();

			worldPosition_ = owner_->GetWorldPosition();
			worldRotation_ = owner_->GetWorldRotation();
			worldScaling_ = owner_->GetWorldScaling();
		}

		worldPosition_ += GetRelativePosition();
		worldRotation_ *= GetRelativeRotation();
		worldScaling_ *= GetRelativeScaling();

		componentToWorldTransformationMatrix_ = componentToWorldTransformationMatrix_ * GetRelativeTransformationMatrix();

		UpdateChildrenComponentToWorldTransformations();
	}

	inline void UpdateChildrenComponentToWorldTransformations()
	{
		std::vector<Component*>::iterator childrenIterator = children_.begin();
		for (; childrenIterator != children_.end(); ++childrenIterator)
		{
			(*childrenIterator)->UpdateComponentToWorldTransformationMatrix();
		}
	}

	Matrix relativeTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix componentToWorldTransformationMatrix_{ Matrix::IdentityMatrix };

	Vector3 pivotPoint_{ Vector3::ZeroVector };

	Quaternion relativeRotation_{ Quaternion::Identity };
	Vector3 relativePosition_{ Vector3::ZeroVector };
	Vector3 relativeScaling_{ Vector3(1.f) };

	// Component to world transformation values
	Quaternion worldRotation_{ Quaternion::Identity };
	Vector3 worldPosition_{ Vector3::ZeroVector };
	Vector3 worldScaling_{ Vector3(1.f) };

	std::vector<Component*> children_;

	ObjectBase* owner_{ nullptr };
	Component* parent_{ nullptr };

	unsigned char isActive_ : 1;
	unsigned char isTickable_ : 1;
	unsigned char isInitialized_ : 1;
private:
};
#endif