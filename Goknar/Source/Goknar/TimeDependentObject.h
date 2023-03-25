#ifndef __TIMEDEPENDENTOBJECT_H__
#define __TIMEDEPENDENTOBJECT_H__

#include "ObjectBase.h"

#include "Goknar/Core.h"

class GOKNAR_API TimeDependentObject
{
public:
	virtual ~TimeDependentObject()
	{

	}

	virtual void Init() {};
	void Tick(float deltaSecond)
	{
		elapsedTime_ += deltaSecond;
		if (timeToRefreshTimeVariables_ < elapsedTime_)
		{
			elapsedTime_ -= timeToRefreshTimeVariables_;
			Operate();
		}
	}

	int GetTicksPerSecond() const
	{
		return ticksPerSecond_;
	}

	void SetTicksPerSecond(int ticksPerSecond)
	{
		ticksPerSecond_ = ticksPerSecond;
		timeToRefreshTimeVariables_ = 1.f / ticksPerSecond;
	}

	bool GetIsActive()
	{
		return isActive_;
	}

	void Activate()
	{
		isActive_ = true;
	}

	void Deactivate()
	{
		isActive_ = false;
		elapsedTime_ = 0.f;
	}

protected:
	TimeDependentObject();

	virtual void Operate() = 0;

	int ticksPerSecond_;
	float timeToRefreshTimeVariables_;
	float elapsedTime_;

	bool isActive_;
};

#endif