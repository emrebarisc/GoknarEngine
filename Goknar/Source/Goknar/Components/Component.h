#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/ObjectBase.h"

class GOKNAR_API Component
{
public:
	Component() = delete;
	Component(ObjectBase* parent);
	virtual ~Component()
	{

	}

	virtual void BeginGame() {}
	virtual void TickComponent(float deltaTime) {}

	bool GetIsTickable() const
	{
		return isTickable_;
	}

	void SetIsTickable(bool isTickable);

	virtual void Destroy();

	ObjectBase* GetParent() const
	{
		return parent_;
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
private:
	ObjectBase* parent_;

	bool isActive_;
	bool isTickable_;
};
#endif