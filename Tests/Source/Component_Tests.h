#pragma once

#include "ITestBase.h"

class Component_Tests : public ITestBase
{
public:
    Component_Tests() = default;
    ~Component_Tests() = default;

    void Run() override final;

private:
    void RunTransformTests();
    void RunPivotTests();
    void RunHierarchyTests();
};