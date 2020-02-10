#ifndef __ObjectBase_H__
#define __ObjectBase_H__

#include <vector>

#include "Core.h"

#include "Math.h"
#include "Matrix.h"

class Component;

/*
    Base object class
*/
class GOKNAR_API ObjectBase
{
public:
    ObjectBase();

	virtual void BeginGame()
	{

	}

    virtual void Tick(float deltaTime)
    {

    }

	void Destroy();

    void SetTickable(bool tickable);
	bool GetTickable()
	{
		return tickable_; 
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

	void AddComponent(Component* component)
	{
		components_.push_back(component);
		totalComponentCount_++;
	}

	Vector3 GetForwardVector()
	{
		return Vector3(worldTransformationMatrix_[0], worldTransformationMatrix_[4], worldTransformationMatrix_[8]);
	}

	Vector3 GetUpVector()
	{
		return Vector3(worldTransformationMatrix_[2], worldTransformationMatrix_[6], worldTransformationMatrix_[10]);
	}

	Vector3 GetLeftVector()
	{
		return Vector3(worldTransformationMatrix_[1], worldTransformationMatrix_[5], worldTransformationMatrix_[9]);
	}

	void SetIsRendered(bool isRendered);

	bool GetIsRendered() const
	{
		return isRendered_;
	}

protected:

private:
    inline void UpdateWorldTransformationMatrix();

	Matrix worldTransformationMatrix_;

	std::vector<Component*> components_;

    Vector3 worldPosition_;
    Vector3 worldRotation_;
    Vector3 worldScaling_;

	int totalComponentCount_;

    bool tickable_;

	bool isRendered_;
};

#endif