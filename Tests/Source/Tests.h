#pragma once

#include <memory>
#include <vector>

#include "ITestBase.h"

class Tests
{
public:
	Tests() = default;
	~Tests() = default;

	void AddTest(const std::shared_ptr<ITestBase>& test)
	{
		tests_.push_back(test);
	}

	void Run();

private:
	std::vector<std::shared_ptr<ITestBase>> tests_;
};