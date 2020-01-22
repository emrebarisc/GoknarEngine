#pragma once

#include "pch.h"

#include "Engine.h"
#include "Log.h"

int main(int argc, char **argv)
{
	Engine *mainEngine = new Engine();

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainEngine->SetApplication(CreateApplication());

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Setting Application: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	mainEngine->Init();

	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	mainEngine->BeginGame();

	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Begin Game Operations: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;
	mainEngine->Run();

	delete mainEngine;
	
	return 0;
}
