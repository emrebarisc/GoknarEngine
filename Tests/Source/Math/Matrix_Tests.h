#pragma once

#include "ITestBase.h"

class Matrix_Tests : public ITestBase
{
public:
    Matrix_Tests() = default;
    ~Matrix_Tests() = default;

    void Run() override final;

private:
    void RunMatrixTests();
    void RunMatrix3x3Tests();
};