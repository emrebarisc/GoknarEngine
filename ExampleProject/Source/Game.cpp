#include "pch.h"

#include "Game.h"

#include <chrono>

#include "Goknar.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Factories/DynamicObjectFactory.h"
#include "Goknar/Helpers/AssetParser.h"
#include "Goknar/Managers/ConfigManager.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Renderer/Renderer.h"

#include "GameState.h"
#include "Characters/DefaultCharacter.h"
#include "Objects/FreeCameraObject.h"
#include "Controllers/FreeCameraController.h"
#include "UI/MainHUD.h"

#include "Managers/ResourceManager.h"
#include "Contents/Image.h"
#include "Materials/MaterialSerializer.h"
#include "Model/StaticMesh.h"

Game::Game() : Application()
{
	REGISTER_CLASS(ObjectBase);
	REGISTER_CLASS(RigidBody);

	engine->SetApplication(this);

	AssetParser::ParseAssets("AssetContainer");

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();

	ConfigManager config;
	std::string configPath = "Config/GameConfig.ini";
	if (config.ReadFile(configPath))
	{
		int width = config.GetInt("Graphics", "WindowWidth", 1920);
		int height = config.GetInt("Graphics", "WindowHeight", 1080);
		engine->GetWindowManager()->SetWindowSize(width, height);

		std::string mainRenderTypeStr = config.GetString("Graphics", "MainRenderType", "Deferred");
		RenderPassType mainRenderType = RenderPassType::Deferred;
		if (mainRenderTypeStr == "Forward")
		{
			mainRenderType = RenderPassType::Forward;
		}

		engine->GetRenderer()->SetMainRenderType(mainRenderType);

		std::string contentDir = config.GetString("Core", "ContentDir", "Content/");
		ContentDir = contentDir;

		std::string mainScenePath = config.GetString("Core", "MainScene", "Scenes/DefaultScene.xml");
		mainScene_->ReadSceneData(mainScenePath);
	}
	else
	{
		GOKNAR_CORE_ERROR("Failed to load %s. Falling back to defaults.", configPath);
	}

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Project is set up in %f seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;
	
	defaultCharacter_ = new DefaultCharacter();
	gameState_ = new GameState();

	engine->SetHUD(new MainHUD());
}

Game::~Game()
{
	delete gameState_;
	gameState_ = nullptr;
}

void Game::PreInit()
{
	Application::PreInit();

	gameState_->PauseGame();
}

void Game::Init()
{
	Application::Init();
}

void Game::PostInit()
{
	Application::PostInit();
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
