#include "pch.h"

#include "TimeDependentObject.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

TimeDependentObject::TimeDependentObject()
	:
	animationElapsedTime_(0.f),
	ticksPerSecond_(30), // Default 30 ticks per second
	timeToRefreshTimeVariables_(1.f / ticksPerSecond_)
{
	engine->RegisterTimeDependentObject(this);
}