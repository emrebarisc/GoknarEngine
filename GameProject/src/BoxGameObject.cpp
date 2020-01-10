#include "BoxGameObject.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Mesh.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"

#include "Goknar/Log.h"


BoxGameObject::BoxGameObject() : ObjectBase()
{
	planeMeshComponent_ = new MeshComponent();
	planeMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(0));

	propellerMeshComponent_ = new MeshComponent();
	propellerMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(1));

	propellerRotationSpeed_ = 0.f;

	SetTickable(true);

	engine->GetInputManager()->AddKeyboardInputDelegate(32, INPUT_ACTION::G_PRESS, std::bind(&BoxGameObject::SpaceKeyDown, this));
}

BoxGameObject::~BoxGameObject()
{
}

void BoxGameObject::BeginGame()
{
	GOKNAR_CORE_INFO("BoxGameObject Begin Game");
}

void BoxGameObject::Tick(float deltaTime)
{
	static float elapsedTime = 0.f;
	elapsedTime += deltaTime;

	Mesh* planeMesh = planeMeshComponent_->GetMesh();
	planeMesh->SetRelativeRotation(planeMesh->GetRelativeRotation() + Vector3(0.f, DEGREE_TO_RADIAN(0.174f * cos(elapsedTime)), 0.f));

	Mesh* propellerMesh = propellerMeshComponent_->GetMesh();
	propellerMesh->SetRelativeRotation(propellerMesh->GetRelativeRotation() + Vector3(0.f, DEGREE_TO_RADIAN(deltaTime * propellerRotationSpeed_), 0.f));
}

void BoxGameObject::SpaceKeyDown()
{
	propellerRotationSpeed_ = 3600.f;
}
