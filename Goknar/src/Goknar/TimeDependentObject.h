#ifndef __TIMEDEPENDENTOBJECT_H__
#define __TIMEDEPENDENTOBJECT_H__

#include "Goknar/Core.h"

class GOKNAR_API TimeDependentObject
{
public:
	TimeDependentObject();
	~TimeDependentObject() {}

	virtual void Init() {};
	void Tick(float deltaSecond)
	{
		elapsedTime_ += deltaSecond;
		if (timeToRefreshTimeVariables_ < elapsedTime_)
		{
			timeToRefreshTimeVariables_ -= elapsedTime_;
			Operate();
		}
	}

protected:
	virtual void Operate() = 0;

	int GetTicksPerSecond() const
	{
		return ticksPerSecond_;
	}

	void SetTicksPerSecond(int ticksPerSecond)
	{
		ticksPerSecond_ = ticksPerSecond;
		timeToRefreshTimeVariables_ = 1.f / ticksPerSecond;
	}

	int ticksPerSecond_;
	float timeToRefreshTimeVariables_;
	float elapsedTime_;
};

#endif