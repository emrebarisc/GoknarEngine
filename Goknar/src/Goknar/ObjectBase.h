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

    void SetTickable(bool tickable);

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

protected:
    Matrix worldTransformationMatrix_;

private:
    inline void UpdateWorldTransformationMatrix();

	std::vector<Component*> components_;

    Vector3 worldPosition_;
    Vector3 worldRotation_;
    Vector3 worldScaling_;

	int totalComponentCount_;

    bool tickable_;
};

#endif