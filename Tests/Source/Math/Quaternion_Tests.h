#pragma once

#include "ITestBase.h"

class Quaternion_Tests : public ITestBase
{
public:
    Quaternion_Tests() = default;
    ~Quaternion_Tests() = default;

    void Run() override final;

private:
    void RunQuaternionTests();
};