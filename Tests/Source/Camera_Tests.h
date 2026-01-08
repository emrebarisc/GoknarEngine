#pragma once

#include "ITestBase.h"

class Camera_Tests : public ITestBase
{
public:
    Camera_Tests() = default;
    ~Camera_Tests() = default;

    void Run() override final;

private:
    void RunInitializationTests();
    void RunMovementTests();
    void RunViewMatrixTests();
    void RunProjectionTests();
};