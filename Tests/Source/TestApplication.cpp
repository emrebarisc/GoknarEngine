#include "TestApplication.h"

#include <Goknar.h>

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

#include "Math/GoknarMath_Tests.h"
#include "Math/Quaternion_Tests.h"
#include "Delegates/Delegates_Tests.h"

TestApplication::TestApplication() :
	Application()
{
	engine->SetApplication(this);

	SetMainScene(new Scene());

	tests.AddTest(std::make_shared<GoknarMath_Tests>());
	tests.AddTest(std::make_shared<Quaternion_Tests>());

	tests.Run();
}

Application *CreateApplication()
{
	return new TestApplication();
}
