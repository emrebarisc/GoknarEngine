#include <Goknar.h>

#include "Goknar/Scene.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Material.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Archer.h"
#include "Dancer.h"
#include "LightController.h"
#include "Sun.h"
#include "Fire.h"
#include "RandomGrassSpawner.h"
#include "MaterialSphereSpawner.h"

#include <chrono>

class Game : public Application
{
public:
	Game();

	~Game()
	{
	}

	void Run() override;

private:
	Archer* archer_;
	Dancer* dancer_;
	LightController* lightController_;
	Sun* sun_;
	Fire* fire_;
	RandomGrassSpawner* randomGrassSpawner_;
	MaterialSphereSpawner* materialSphereSpawner_;
};

Game::Game() : Application()
{
	engine->SetApplication(this);

	engine->GetRenderer()->SetMainRenderType(RenderPassType::Deferred);

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/Scene_2.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	archer_ = new Archer();
	// dancer_ = new Dancer();
	// lightController_ = new LightController();
	sun_ = new Sun();
	// randomGrassSpawner_ = new RandomGrassSpawner();
	fire_ = new Fire();
	materialSphereSpawner_ = new MaterialSphereSpawner();

	MeshUnit* floorStaticMesh = engine->GetResourceManager()->GetContent<MeshUnit>("Meshes/SM_Floor.fbx");
	if (floorStaticMesh)
	{
		floorStaticMesh->GetMaterial()->SetShadingModel(MaterialShadingModel::TwoSided);
	}

	engine->GetWindowManager()->SetIsInFullscreen(false);
	//engine->SetTimeScale(0.1f);
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
