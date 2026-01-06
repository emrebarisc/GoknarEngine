#pragma once

class ITestBase
{
public:
	ITestBase() = default;
	virtual ~ITestBase() = default;

	virtual void Run() = 0;
};