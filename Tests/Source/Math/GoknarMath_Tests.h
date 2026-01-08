#pragma once

#include "ITestBase.h"

class GoknarMath_Tests : public ITestBase
{
public:
    GoknarMath_Tests() = default;
    ~GoknarMath_Tests() = default;

    void Run() override final;

private:
    void RunMacroTests();
    void RunMathUtilsTests();
    void RunVector2Tests();
    void RunVector3Tests();
    void RunVector4Tests();
    void RunStaticMathTests();
};