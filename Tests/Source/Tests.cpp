#include "Tests.h"

void Tests::Run()
{
	for(auto test : tests_)
	{
		test->Run();
	}
}