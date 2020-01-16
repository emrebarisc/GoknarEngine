#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Engine.h"

ObjectBase::ObjectBase() : tickable_(false)
{
	engine->RegisterObject(this);

	worldPosition_ = Vector3::ZeroVector;
	worldRotation_ = Vector3::ZeroVector;
	worldScaling_ = Vector3::ZeroVector;
}

void ObjectBase::SetTickable(bool tickable)
{
	tickable_ = tickable;
	if (tickable_)
	{
		engine->AddToTickableObjects(this);
	}
}

void ObjectBase::SetWorldPosition(const Vector3& position)
{
}

void ObjectBase::SetWorldRotation(const Vector3& rotation)
{
}

void ObjectBase::SetWorldScaling(const Vector3& scaling)
{
}
