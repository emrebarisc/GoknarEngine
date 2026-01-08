#include "Delegates_Tests.h"

#include "DelegateDemo.h"
#include "ArticleSamples.h"
#include "Benchmark.h"

void Delegates_Tests::Run()
{
    try
    {
        ArticleSampleSet().Demo();
        DelegateSample::Demo();
        Benchmark::Run();

        GOKNAR_CORE_INFO("All Delegate test sequences completed.");
    }
    catch (...)
    {

        GOKNAR_CORE_ERROR("An unexpected error occurred during testing Delegates.");
    }
}