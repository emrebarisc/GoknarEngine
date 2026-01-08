#include "TestApplication.h"

#include <Goknar.h>

#include "Goknar/Engine.h"

#include "Camera_Tests.h"
#include "Math/GoknarMath_Tests.h"
#include "Math/InterpolatingValue_Tests.h"
#include "Math/Matrix_Tests.h"
#include "Math/Quaternion_Tests.h"
#include "Delegates/Delegates_Tests.h"

TestApplication::TestApplication() :
	Application()
{
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("###############                              TEST CASES                              ###############");
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("");
	GOKNAR_CORE_INFO("");
	GOKNAR_CORE_INFO("");

	tests.AddTest(std::make_shared<Camera_Tests>());

	//tests.AddTest(std::make_shared<Delegates_Tests>());

	tests.AddTest(std::make_shared<GoknarMath_Tests>());
	tests.AddTest(std::make_shared<InterpolatingValue_Tests>());
	tests.AddTest(std::make_shared<Matrix_Tests>());
	tests.AddTest(std::make_shared<Quaternion_Tests>());

	tests.Run();

	GOKNAR_CORE_INFO("");
	GOKNAR_CORE_INFO("");
	GOKNAR_CORE_INFO("");
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("####################################################################################################");
	GOKNAR_CORE_INFO("####################################################################################################");

	shutdownTimer.SetTimeToTick(1.f);
	shutdownTimer.CallOnTick([]() { engine->Exit(); });
}

Application *CreateApplication()
{
	return new TestApplication();
}
