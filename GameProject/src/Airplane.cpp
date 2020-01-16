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
}

void Airplane::BeginGame()
{
	GOKNAR_CORE_INFO("Airplane Begin Game");
}

void Airplane::Tick(float deltaTime)
{
	static float elapsedTime = 0.f;
	elapsedTime += deltaTime;

	Mesh* planeMesh = planeMeshComponent_->GetMesh();
	float airplaneRotation = DEGREE_TO_RADIAN(0.174f * cos(elapsedTime));
	planeMesh->SetRelativePosition(planeMesh->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	planeMesh->SetRelativeRotation(planeMesh->GetRelativeRotation() + Vector3(0.f, airplaneRotation, 0.f));

	Mesh* propellerMesh = propellerMeshComponent_->GetMesh();
	propellerMesh->SetRelativePosition(propellerMesh->GetRelativePosition() + Vector3(0.0125f * cos(elapsedTime), 0.f, 0.f));
	propellerMesh->SetRelativeRotation(propellerMesh->GetRelativeRotation() + Vector3(0.f, airplaneRotation + DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));
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
