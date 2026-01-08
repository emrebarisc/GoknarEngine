#pragma once

#include "ITestBase.h"

class InterpolatingValue_Tests : public ITestBase
{
public:
    InterpolatingValue_Tests() = default;
    ~InterpolatingValue_Tests() = default;

    void Run() override final;

private:
    void RunBasicInterpolationTests();
    void RunTwoDestinationInterpolationTests();
    void RunRotatingInterpolationTests();
};