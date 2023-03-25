#include "pch.h"

#include "Component.h"
#include "Engine.h"

Component::Component(ObjectBase* parent) :
	parent_(parent),
	isActive_(true),
	isTickable_(false)
{
	parent->AddComponent(this);
	engine->RegisterComponent(this);
}

void Component::Destroy()
{
	delete this;
}

void Component::SetIsTickable(bool isTickable)
{
	isTickable_ = isTickable;
	if (isTickable_)
	{
		engine->AddToTickableComponents(this);
	}
	else
	{
		engine->RemoveFromTickableComponents(this);
	}
}

