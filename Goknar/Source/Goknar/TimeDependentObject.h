#ifndef __TIMEDEPENDENTOBJECT_H__
#define __TIMEDEPENDENTOBJECT_H__

#include "ObjectBase.h"

#include "Goknar/Core.h"

class GOKNAR_API TimeDependentObject
{
public:
	virtual ~TimeDependentObject();

	virtual void Tick(float deltaSecond) = 0;

	bool GetIsActive() const
	{
		return isActive_;
	}

	virtual void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

protected:
	TimeDependentObject();

	bool isActive_{ true };
};

#endif