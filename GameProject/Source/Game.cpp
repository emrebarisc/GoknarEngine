#include "pch.h"

#include "Game.h"

#include <Goknar.h>

#include "Goknar/Scene.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Archer.h"
#include "ArcherCharacter.h"
#include "Dancer.h"
#include "LightController.h"
#include "Sun.h"
#include "Fire.h"
#include "RandomGrassSpawner.h"
#include "MaterialSphereSpawner.h"
#include "PhysicsObjectSpawner.h"
#include "Objects/FreeCameraObject.h"
#include "Objects/Terrain.h"
#include "Objects/PhysicsBox.h"

#include <chrono>

Game::Game() : Application()
{
	engine->SetApplication(this);

	engine->GetRenderer()->SetMainRenderType(RenderPassType::Deferred);

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/Scene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	sun_ = new Sun();
	//dancer_ = new Dancer();
	//lightController_ = new LightController();
	//randomGrassSpawner_ = new RandomGrassSpawner();
	//fire_ = new Fire();
	//materialSphereSpawner_ = new MaterialSphereSpawner();


	//MeshUnit* floorStaticMesh = engine->GetResourceManager()->GetContent<MeshUnit>("Meshes/SM_Floor.fbx");
	//if (floorStaticMesh)
	//{
	//	floorStaticMesh->GetMaterial()->SetShadingModel(MaterialShadingModel::TwoSided);
	//}

	//physicsArcher_ = new ArcherCharacter();
	freeCameraObject_ = new FreeCameraObject();

	physicsObjectSpawner_ = new PhysicsObjectSpawner();

	terrain = new Terrain();

	engine->GetWindowManager()->SetWindowSize(1920, 1080);
	engine->GetWindowManager()->SetIsInFullscreen(false);
	//engine->SetTimeScale(0.1f);

	//boxFloor_ = new PhysicsBox();
	//boxFloor_->SetWorldPosition(Vector3{0.f, 0.f, -10.f});
	//boxFloow_->SetWorldScaling(Vector3{50.f, 50.f, 0.1f});
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
