#include "pch.h"

#include "Controller.h"

#include "Engine.h"

Controller::Controller()
{
	if (!engine->GetController())
	{
		engine->SetController(this);
	}
}

Controller::~Controller()
{
	if (engine->GetController() == this)
	{
		engine->SetController(nullptr);
	}
}