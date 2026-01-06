#pragma once

#include "Goknar/Application.h"

#include "Tests.h"

class GOKNAR_API TestApplication : public Application
{
public:
	TestApplication();

	~TestApplication() = default;

private:
	Tests tests;
};