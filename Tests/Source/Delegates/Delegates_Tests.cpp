#include "Delegates_Tests.h"

#include "DelegateDemo.h"
#include "ArticleSamples.h"
#include "Benchmark.h"

void Delegates_Tests::Run()
{
	ArticleSampleSet().Demo();
	DelegateSample::Demo();
	Benchmark::Run();
}