#include "Airplane.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"

#include "Goknar/Log.h"


Airplane::Airplane() : ObjectBase()
{
	planeMeshComponent_ = new MeshComponent(this);
	planeMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(0));

	propellerMeshComponent_ = new MeshComponent(this);
	propellerMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(1));

	propellerRotationSpeed_ = 0.f;
	moveRightSpeed_ = 30.f;
	moveLefttSpeed_ = 30.f;

	SetTickable(true);

	engine->GetInputManager()->AddKeyboardInputDelegate(32, INPUT_ACTION::G_PRESS, std::bind(&Airplane::SpaceKeyDown, this));
}

Airplane::~Airplane()
{
	delete planeMeshComponent_;
	delete propellerMeshComponent_;
}

void Airplane::BeginGame()
{
	GOKNAR_CORE_INFO("Airplane Begin Game");
	SetWorldPosition(Vector3(0.f, 0.f, 20.f));
}

void Airplane::Tick(float deltaTime)
{
	static float elapsedTime = 0.f;
	elapsedTime += deltaTime;

	float airplaneRotation = DEGREE_TO_RADIAN(0.174f * cos(elapsedTime));
	planeMeshComponent_->SetRelativePosition(planeMeshComponent_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	planeMeshComponent_->SetRelativeRotation(planeMeshComponent_->GetRelativeRotation() + Vector3(0.f, airplaneRotation, 0.f));

	propellerMeshComponent_->SetRelativePosition(propellerMeshComponent_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	propellerMeshComponent_->SetRelativeRotation(propellerMeshComponent_->GetRelativeRotation() + Vector3(0.f, airplaneRotation + DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));

	SetWorldPosition(GetWorldPosition() + Vector3(0.f, deltaTime, 0.f));

	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();
	Vector3 cameraPosition = mainCamera->GetPosition();
	mainCamera->SetPosition(Vector3(cameraPosition.x, GetWorldPosition().y, cameraPosition.z));
}

void Airplane::SpaceKeyDown()
{
	propellerRotationSpeed_ = propellerRotationSpeed_ > 0 ? 0 : 3600.f;
}

void Airplane::GoLeft()
{

}

void Airplane::GoRight()
{
}
