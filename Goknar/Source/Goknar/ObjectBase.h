#ifndef __ObjectBase_H__
#define __ObjectBase_H__

#include <vector>

#include "Core.h"

#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

class Component;
class SocketComponent;

/*
    Base object class
*/
class GOKNAR_API ObjectBase
{
public:
    ObjectBase();
	virtual ~ObjectBase();

	virtual void Init();

	virtual void BeginGame()
	{

	}

    virtual void Tick(float deltaTime)
    {

    }
	
	void Destroy();

	Component* GetRootComponent() const
	{
		return rootComponent_;
	}

	void SetRootComponent(Component* rootComponent);

    void SetIsTickable(bool tickable);
	bool GetIsTickable() const
	{
		return isTickable_; 
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	void SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix = true);
	const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	virtual void SetWorldRotation(const Quaternion& rotation, bool updateWorldTransformationMatrix = true);
	const Quaternion& GetWorldRotation() const
	{
		return worldRotation_;
	}

	void SetWorldScaling(const Vector3& scaling, bool updateWorldTransformationMatrix = true);
	const Vector3& GetWorldScaling() const
	{
		return worldScaling_;
	}

	const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	Vector3 GetForwardVector() const
	{
		return worldTransformationMatrix_.GetForwardVector().GetNormalized();
	}

	Vector3 GetLeftVector() const
	{
		return worldTransformationMatrix_.GetLeftVector().GetNormalized();
	}

	Vector3 GetUpVector() const
	{
		return worldTransformationMatrix_.GetUpVector().GetNormalized();
	}

	void SetIsActive(bool isRendered);

	bool GetIsActive() const
	{
		return isActive_;
	}

	template<class T>
	T* AddSubComponent();

	void AttachToSocket(SocketComponent* socketComponent);
	void RemoveFromSocket(SocketComponent* socketComponent);

	ObjectBase* GetParent() const
	{
		return parent_;
	}

	void SetParent(ObjectBase* parent, bool resetChildTransformation = false, bool updateWorldTransformation = true);

	void AddChild(ObjectBase* child)
	{
		children_.push_back(child);
	}
	void RemoveChild(ObjectBase* child);

	Vector3 GetRelativePositionInWorldSpace(const Vector3& relativePosition);
	Vector3 GetWorldPositionInRelativeSpace(const Vector3& positionInWorldSpace);
	Vector3 GetRelativeDirectionInWorldSpace(const Vector3& relativeDirection);
	Vector3 GetWorldDirectionInRelativeSpace(const Vector3& directionInWorldSpace);

protected:
	void AddComponent(Component* component);

	virtual void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix);
	virtual void UpdateWorldTransformationMatrix();
	virtual void UpdateChildrenTransformations();

	Matrix worldTransformationMatrix_{ Matrix::IdentityMatrix };

	Quaternion worldRotation_{ Quaternion::Identity };
	Vector3 worldPosition_{ Vector3::ZeroVector };
	Vector3 worldScaling_{ Vector3(1.f) };

private:
	std::vector<ObjectBase*> children_;
	std::vector<Component*> components_;
	Component* rootComponent_{ nullptr };

	ObjectBase* parent_{ nullptr };
	SocketComponent* parentSocket_{ nullptr };

	int totalComponentCount_;

    unsigned int isTickable_ : 1;
	unsigned int isActive_ : 1;
	unsigned int isInitialized_ : 1;
};

template<class T>
T* ObjectBase::AddSubComponent()
{
	T* subComponent = new T(rootComponent_);
	AddComponent(subComponent);

	return subComponent;
}

#endif