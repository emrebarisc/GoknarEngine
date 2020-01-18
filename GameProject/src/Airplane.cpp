#include "Airplane.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
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

	planeMeshComponent2_ = new MeshComponent(this);
	planeMeshComponent2_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(0));
	planeMeshComponent2_->SetRelativePosition(Vector3(-5.f, 0.f, 0.f));

	planeMeshComponent3_ = new MeshComponent(this);
	planeMeshComponent3_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(0));
	planeMeshComponent3_->SetRelativePosition(Vector3(5.f, 0.f, 0.f));

	propellerMeshComponent_ = new MeshComponent(this);
	propellerMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(1));

	propellerMeshComponent2_ = new MeshComponent(this);
	propellerMeshComponent2_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(1));
	propellerMeshComponent2_->SetRelativePosition(Vector3(-5.f, 0.f, 0.f));

	propellerMeshComponent3_ = new MeshComponent(this);
	propellerMeshComponent3_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(1));
	propellerMeshComponent3_->SetRelativePosition(Vector3(5.f, 0.f, 0.f));

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
}

void Airplane::Tick(float deltaTime)
{
	static float elapsedTime = 0.f;
	elapsedTime += deltaTime;

	float airplaneRotation = DEGREE_TO_RADIAN(0.174f * cos(elapsedTime));
	planeMeshComponent_->SetRelativePosition(planeMeshComponent_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	planeMeshComponent_->SetRelativeRotation(planeMeshComponent_->GetRelativeRotation() + Vector3(0.f, airplaneRotation, 0.f));

	planeMeshComponent2_->SetRelativePosition(planeMeshComponent2_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	planeMeshComponent2_->SetRelativeRotation(planeMeshComponent2_->GetRelativeRotation() + Vector3(0.f, airplaneRotation, 0.f));

	planeMeshComponent3_->SetRelativePosition(planeMeshComponent3_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	planeMeshComponent3_->SetRelativeRotation(planeMeshComponent3_->GetRelativeRotation() + Vector3(0.f, airplaneRotation, 0.f));

	propellerMeshComponent_->SetRelativePosition(propellerMeshComponent_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	propellerMeshComponent_->SetRelativeRotation(propellerMeshComponent_->GetRelativeRotation() + Vector3(0.f, airplaneRotation + DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));

	propellerMeshComponent2_->SetRelativePosition(propellerMeshComponent2_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	propellerMeshComponent2_->SetRelativeRotation(propellerMeshComponent2_->GetRelativeRotation() + Vector3(0.f, airplaneRotation + DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));

	propellerMeshComponent3_->SetRelativePosition(propellerMeshComponent3_->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	propellerMeshComponent3_->SetRelativeRotation(propellerMeshComponent3_->GetRelativeRotation() + Vector3(0.f, airplaneRotation + DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));

	SetWorldPosition(GetWorldPosition() + Vector3(0.f, deltaTime, 0.f));
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
