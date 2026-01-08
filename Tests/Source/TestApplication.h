#pragma once

#include "Goknar/Application.h"
#include "Goknar/Timer.h"

#include "Tests.h"

class GOKNAR_API TestApplication : public Application
{
public:
	TestApplication();

	~TestApplication() = default;

private:
	Tests tests;

	Timer shutdownTimer;
};