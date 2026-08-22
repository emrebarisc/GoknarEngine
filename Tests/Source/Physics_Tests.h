#pragma once

#include "ITestBase.h"

class Physics_Tests : public ITestBase
{
public:
	void Run() override final;

private:
	void RunCollisionScaleRecoveryTests();
};
