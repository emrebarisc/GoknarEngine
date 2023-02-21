#pragma once

#ifdef GOKNAR_PLATFORM_WINDOWS
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#ifdef _DEBUG
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
		// allocations to be of _CLIENT_BLOCK type
	#else
		#define DBG_NEW new
	#endif
#else
	#define DBG_NEW new
#endif

#include <stdlib.h>

#include "pch.h"

#include "Engine.h"
#include "Log.h"

int main(int argc, char **argv)
{
#ifdef GOKNAR_PLATFORM_WINDOWS
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);

	_CrtMemState oldMemoryState;
	_CrtMemCheckpoint(&oldMemoryState);
#endif

	Engine* mainEngine = DBG_NEW Engine();

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

#ifdef GOKNAR_PLATFORM_WINDOWS
	//_CrtMemState newMemoryState;
	//_CrtMemCheckpoint(&newMemoryState);

	//_CrtMemState stateDiff;
	//if (_CrtMemDifference(&stateDiff, &oldMemoryState, &newMemoryState))
	//{
	//	//OutputDebugString("-----------_CrtMemDumpStatistics ---------");
	//	//_CrtMemDumpStatistics(&stateDiff);
	//	//OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------");
	//	//_CrtMemDumpAllObjectsSince(&oldMemoryState);
	//	OutputDebugString("-----------_CrtDumpMemoryLeaks ---------");
	//	_CrtDumpMemoryLeaks();
	//}
#endif

	return 0;
}
