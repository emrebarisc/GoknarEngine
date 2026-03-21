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

    virtual void Tick(float)
    {

    }
	
	virtual void Destroy();

	inline Component* GetRootComponent() const
	{
		return rootComponent_;
	}

	void SetRootComponent(Component* rootComponent);
	
    virtual void SetIsTickable(bool tickable);
	inline bool GetIsTickable() const
	{
		return isTickable_; 
	}

	inline bool GetIsTickEnabled() const
	{
		return isTickEnabled_;
	}

	inline void SetIsTickEnabled(bool isTickEnabled)
	{
		isTickEnabled_ = isTickEnabled;
	}

	inline bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	virtual void SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix = true);
	inline const Vector3& GetWorldPosition() const
	{
		return worldPosition_;
	}

	virtual void SetWorldRotation(const Quaternion& rotation, bool updateWorldTransformationMatrix = true);
	inline const Quaternion& GetWorldRotation() const
	{
		return worldRotation_;
	}

	virtual void SetWorldScaling(const Vector3& scaling, bool updateWorldTransformationMatrix = true);
	inline const Vector3& GetWorldScaling() const
	{
		return worldScaling_;
	}

	inline const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	inline const Matrix& GetWorldTransformationMatrixWithoutScaling() const
	{
		return worldTransformationMatrixWithoutScaling_;
	}

	inline Vector3 GetForwardVector() const
	{
		return worldTransformationMatrix_.GetForwardVector().GetNormalized();
	}

	inline Vector3 GetLeftVector() const
	{
		return worldTransformationMatrix_.GetLeftVector().GetNormalized();
	}

	inline Vector3 GetUpVector() const
	{
		return worldTransformationMatrix_.GetUpVector().GetNormalized();
	}

	inline std::string GetName() const
	{
		return std::to_string(GUID_) + "_" + name_;
	}

	inline std::string GetNameWithoutId() const
	{
		return name_;
	}

	inline void SetName(const std::string& name)
	{
		name_ = name;
	}

	virtual void SetIsActive(bool isActive);

	inline bool GetIsActive() const
	{
		return isActive_;
	}

	template<class T>
	T* AddSubComponent();

	template<class T>
	T* GetFirstComponentOfType();
	
	template<class T>
	std::vector<T*> GetComponentsOfType();

	inline const std::vector<Component*>& GetComponents() const
	{
		return components_;
	}

	void AttachToSocket(SocketComponent* socketComponent);
	void RemoveFromSocket(SocketComponent* socketComponent);

	inline ObjectBase* GetParent() const
	{
		return parent_;
	}

	void SetParent(ObjectBase* parent, SnappingRule snappingRule = SnappingRule::KeepWorldAll, bool updateWorldTransformation = true);

	inline void AddChild(ObjectBase* child)
	{
		children_.push_back(child);
	}
	void RemoveChild(ObjectBase* child);
	inline const std::vector<ObjectBase*>& GetChildren() const
	{
		return children_;
	}

	Vector3 GetRelativePositionInWorldSpace(const Vector3& relativePosition);
	Vector3 GetWorldPositionInRelativeSpace(const Vector3& positionInWorldSpace);
	Vector3 GetRelativeDirectionInWorldSpace(const Vector3& relativeDirection);
	Vector3 GetWorldDirectionInRelativeSpace(const Vector3& directionInWorldSpace);

	inline int GetGUID() const
	{
		return GUID_;
	}

protected:
	virtual void AddComponent(Component* component);
	virtual void DestroyInner();

	virtual void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix);
	virtual void UpdateWorldTransformationMatrix();
	virtual void UpdateChildrenTransformations();

	bool GetIsPendingDestroy() const
	{
		return isPendingDestroy_;
	}

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