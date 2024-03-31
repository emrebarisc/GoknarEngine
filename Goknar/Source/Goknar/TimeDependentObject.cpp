#include "pch.h"

#include "TimeDependentObject.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

TimeDependentObject::TimeDependentObject()
{
	engine->RegisterTimeDependentObject(this);
}

TimeDependentObject::~TimeDependentObject()
{
	engine->RemoveTimeDependentObject(this);
}