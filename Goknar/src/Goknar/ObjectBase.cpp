#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Engine.h"

ObjectBase::ObjectBase() : tickable_(false)
{
	engine->RegisterObject(this);
}

void ObjectBase::SetTickable(bool tickable)
{
	tickable_ = tickable;
	engine->AddToTickableObjects(this);
}