#pragma once

#include "ITestBase.h"

class ObjectBase_Tests : public ITestBase
{
public:
    ObjectBase_Tests() = default;
    ~ObjectBase_Tests() = default;

    void Run() override final;

private:
    void RunHierarchyTests();
    void RunSnappingTests();
    void RunComponentManagementTests();
};