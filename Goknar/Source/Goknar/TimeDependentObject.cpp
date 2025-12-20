#include "pch.h"

#include "TimeDependentObject.h"

#include "Engine.h"

TimeDependentObject::TimeDependentObject()
{
	engine->RegisterTimeDependentObject(this);
}

TimeDependentObject::~TimeDependentObject()
{
	engine->RemoveTimeDependentObject(this);
}