#include "pch.h"

#include "Timer.h"

Timer::Timer() : TimeDependentObject()
{

}

void Timer::Tick(float deltaSecond)
{
	elapsedTime_ += deltaSecond;
	if (timeToRefreshTimeVariables_ < elapsedTime_)
	{
		elapsedTime_ -= timeToRefreshTimeVariables_;
		Operate();
	}
}

void Timer::SetIsActive(bool isActive)
{
	TimeDependentObject::SetIsActive(isActive);
	if (!isActive)
	{
		elapsedTime_ = 0.f;
	}
}