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
		animationElapsedTime_ += deltaSecond;
		if (timeToRefreshTimeVariables_ < animationElapsedTime_)
		{
			animationElapsedTime_ -= timeToRefreshTimeVariables_;
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

protected:
	TimeDependentObject();

	virtual void Operate() = 0;

	int ticksPerSecond_;
	float timeToRefreshTimeVariables_;
	float animationElapsedTime_;
};

#endif