#include "pch.h"

#include "Controller.h"

#include "Engine.h"

Controller::Controller() : 
	ObjectBase()
{
	SetName("Controller");
}

Controller::~Controller()
{
}

void Controller::PreInit()
{
	ObjectBase::PreInit();
}

void Controller::Init()
{
	ObjectBase::Init();
}

void Controller::PostInit()
{
	SetupInputs();

	ObjectBase::PostInit();
}
