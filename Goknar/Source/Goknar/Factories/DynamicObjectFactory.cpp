#include "pch.h"

#include "DynamicObjectFactory.h"

#include "Goknar/ObjectBase.h"
#include "Goknar/Objects/ReflectionProbeObject.h"
#include "Goknar/Physics/RigidBody.h"

DynamicObjectFactory* DynamicObjectFactory::instance_ = nullptr;

void DynamicObjectFactory::RegisterBuiltInClasses()
{
	static bool areBuiltInClassesRegistered = false;
	if (areBuiltInClassesRegistered || instance_ == nullptr)
	{
		return;
	}

	instance_->RegisterClass("ReflectionProbeObject", []() -> ReflectionProbeObject*
		{
			return new ReflectionProbeObject();
		});

	instance_->RegisterClass("ObjectBase", []() -> ObjectBase*
		{
			return new ObjectBase();
		});

	instance_->RegisterClass("RigidBody", []() -> ObjectBase*
		{
			return new RigidBody();
		});

	areBuiltInClassesRegistered = true;
}
