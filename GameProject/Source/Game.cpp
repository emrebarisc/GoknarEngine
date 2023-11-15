#include <Goknar.h>

#include "Goknar/Scene.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Material.h"
#include "Goknar/Managers/ResourceManager.h"

#include "Archer.h"
#include "Dancer.h"
#include "LightController.h"
#include "Sun.h"
#include "Fire.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/WindowManager.h"

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
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/Scene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	archer_ = new Archer();
	dancer_ = new Dancer();
	lightController_ = new LightController();
	sun_ = new Sun();
	fire_ = new Fire();

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
