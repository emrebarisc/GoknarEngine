#ifndef __ObjectBase_H__
#define __ObjectBase_H__

#include <vector>

#include "Core.h"

#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

class Component;
class Engine;
class SocketComponent;

enum class SnappingRule : unsigned char
{
	None = 								0b00000000,
	KeepWorldPosition = 				0b00000001,
	KeepWorldRotation = 				0b00000010,
	KeepWorldScaling = 					0b00000100,
	KeepWorldPositionRotation = 		KeepWorldPosition | KeepWorldRotation,
	KeepWorldPositionScaling = 			KeepWorldPosition | KeepWorldScaling,
	KeepWorldScalingRotation = 			KeepWorldRotation | KeepWorldScaling,
	KeepWorldAll =						KeepWorldPosition | KeepWorldRotation | KeepWorldScaling
};

struct GOKNAR_API ObjectInitializer
{
	ObjectInitializer() = default;
	ObjectInitializer(bool initOnConstr) : setForInitializeOnConstructor(initOnConstr) {}

	bool setForInitializeOnConstructor{ true };
};

class GOKNAR_API ObjectBase
{
	friend Engine;
public:
	ObjectBase(const ObjectInitializer& objectInitializer = ObjectInitializer());
	virtual ~ObjectBase();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void BeginGame()
	{

	}

    virtual void Tick(float deltaTime)
    {

    }
	
	virtual void Destroy();

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

	bool GetIsTickEnabled() const
	{
		return isTickEnabled_;
	}

	void SetIsTickEnabled(bool isTickEnabled)
	{
		isTickEnabled_ = isTickEnabled;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	virtual void SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix = true);
	const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	virtual void SetWorldRotation(const Quaternion& rotation, bool updateWorldTransformationMatrix = true);
	const Quaternion& GetWorldRotation() const
	{
		return worldRotation_;
	}

	virtual void SetWorldScaling(const Vector3& scaling, bool updateWorldTransformationMatrix = true);
	const Vector3& GetWorldScaling() const
	{
		return worldScaling_;
	}

	const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	const Matrix& GetWorldTransformationMatrixWithoutScaling() const
	{
		return worldTransformationMatrixWithoutScaling_;
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

	std::string GetName() const
	{
		return name_ + std::to_string(GUID_);
	}

	std::string GetNameWithoutId() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	virtual void SetIsActive(bool isActive);

	bool GetIsActive() const
	{
		return isActive_;
	}

	template<class T>
	T* AddSubComponent();

	template<class T>
	T* GetFirstComponentOfType();
	
	template<class T>
	std::vector<T*> GetComponentsOfType();

	const std::vector<Component*>& GetComponents() const
	{
		return components_;
	}

	void AttachToSocket(SocketComponent* socketComponent);
	void RemoveFromSocket(SocketComponent* socketComponent);

	ObjectBase* GetParent() const
	{
		return parent_;
	}

	void SetParent(ObjectBase* parent, SnappingRule snappingRule = SnappingRule::KeepWorldAll, bool updateWorldTransformation = true);

	void AddChild(ObjectBase* child)
	{
		children_.push_back(child);
	}
	void RemoveChild(ObjectBase* child);

	Vector3 GetRelativePositionInWorldSpace(const Vector3& relativePosition);
	Vector3 GetWorldPositionInRelativeSpace(const Vector3& positionInWorldSpace);
	Vector3 GetRelativeDirectionInWorldSpace(const Vector3& relativeDirection);
	Vector3 GetWorldDirectionInRelativeSpace(const Vector3& directionInWorldSpace);

	int GetGUID() const
	{
		return GUID_;
	}

protected:
	virtual void AddComponent(Component* component);
	virtual void DestroyInner();

	virtual void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix);
	virtual void UpdateWorldTransformationMatrix();
	virtual void UpdateChildrenTransformations();

	Matrix worldTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix worldTransformationMatrixWithoutScaling_{ Matrix::IdentityMatrix };

	Quaternion worldRotation_{ Quaternion::Identity };
	Vector3 worldPosition_{ Vector3::ZeroVector };
	Vector3 worldScaling_{ Vector3(1.f) };

private:
	std::string name_{ "ObjectBase" };
	std::vector<ObjectBase*> children_;
	std::vector<Component*> components_;
	Component* rootComponent_{ nullptr };

	ObjectBase* parent_{ nullptr };
	SocketComponent* parentSocket_{ nullptr };

	int totalComponentCount_;

	unsigned int GUID_{ 0 };
    unsigned char isTickable_ : 1;
    unsigned char isTickEnabled_ : 1;
	unsigned char isActive_ : 1;
	unsigned char isInitialized_ : 1;
	unsigned char isPendingDestroy_ : 1;
};

template<class T>
T* ObjectBase::AddSubComponent()
{
	T* subComponent = new T(rootComponent_);
	AddComponent(subComponent);

	return subComponent;
}

template<class T>
T* ObjectBase::GetFirstComponentOfType()
{
	std::vector<Component *>::iterator componentIterator = components_.begin();
	for (; componentIterator != components_.end(); ++componentIterator)
	{
		T* component = dynamic_cast<T*>(*componentIterator);
		if(component != nullptr)
		{
			return component;
		}
	}
	
	return nullptr;
}

template<class T>
std::vector<T*> ObjectBase::GetComponentsOfType()
{
	std::vector<T*> components;

	std::vector<Component *>::iterator componentIterator = components_.begin();
	for (; componentIterator != components_.end(); ++componentIterator)
	{
		T* component = dynamic_cast<T*>(*componentIterator);
		if(component != nullptr)
		{
			components.push_back(component);
		}
	}
	
	return components;
}

#endif