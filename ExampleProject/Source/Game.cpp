#include "pch.h"

#include "Game.h"

#include <chrono>

#include "Goknar.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Factories/DynamicObjectFactory.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Renderer/Renderer.h"

#include "GameState.h"
#include "Characters/DefaultCharacter.h"
#include "Data/MaterialInitializer.h"
#include "Objects/FreeCameraObject.h"
#include "Controllers/FreeCameraController.h"

Game::Game() : Application()
{
	REGISTER_CLASS(ObjectBase);
	REGISTER_CLASS(RigidBody);

	MaterialInitializer::Init();

	engine->SetApplication(this);

	engine->GetRenderer()->SetMainRenderType(RenderPassType::Deferred);

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/DefaultScene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;
	
	//freeCameraObject_ = new FreeCameraObject();
	//freeCameraObject_->SetWorldPosition(Vector3{ 0.f, 0.f, 1.f });
	//freeCameraObject_->GetFreeCameraController()->SetIsActive(true);

	defaultCharacter_ = new DefaultCharacter();

	engine->GetWindowManager()->SetWindowSize(1600, 900);
	engine->GetWindowManager()->SetIsInFullscreen(false);

	gameState_ = new GameState();
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
