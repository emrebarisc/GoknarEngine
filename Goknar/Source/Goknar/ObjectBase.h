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

	void SetWorldPosition(const Vector3& position);
	const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	void SetWorldRotation(const Quaternion& rotation);
	const Quaternion& GetWorldRotation() const
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

	Vector3 GetForwardVector() const
	{
		return Vector3(worldTransformationMatrix_[0], worldTransformationMatrix_[4], worldTransformationMatrix_[8]).GetNormalized();
	}

	Vector3 GetUpVector() const
	{
		return Vector3(worldTransformationMatrix_[2], worldTransformationMatrix_[6], worldTransformationMatrix_[10]).GetNormalized();
	}

	Vector3 GetLeftVector() const
	{
		return Vector3(worldTransformationMatrix_[1], worldTransformationMatrix_[5], worldTransformationMatrix_[9]).GetNormalized();
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

protected:

private:
	void AddComponent(Component* component);

	virtual void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix);
    virtual void UpdateWorldTransformationMatrix();
	virtual void UpdateChildrenTransformations();

	Matrix worldTransformationMatrix_{ Matrix::IdentityMatrix };

	std::vector<ObjectBase*> children_;
	std::vector<Component*> components_;
	Component* rootComponent_{ nullptr };

	Quaternion worldRotation_{ Quaternion::Identity };
	Vector3 worldPosition_{ Vector3::ZeroVector };
	Vector3 worldScaling_{ Vector3(1.f) };

	ObjectBase* parent_{ nullptr };

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