#include "BoxGameObject.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Mesh.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"

#include "Goknar/Log.h"

#include "../../Goknar/outsourced/GLFW/GLFW/include/GLFW/glfw3.h"

BoxGameObject::BoxGameObject() : ObjectBase()
{
	boxMeshComponent_ = new MeshComponent();
	boxMeshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(0));
	SetTickable(true);

	engine->GetInputManager()->AddKeyboardInputDelegate(GLFW_KEY_SPACE, INPUT_ACTION::G_PRESS, std::bind(&BoxGameObject::SpaceKeyDown, this));
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

	Mesh* boxMesh = boxMeshComponent_->GetMesh();
	boxMesh->SetRelativePosition(boxMesh->GetRelativePosition() + Vector3(0.f, 0.f, 0.05f * cos(elapsedTime)));
	boxMesh->SetRelativeRotation(boxMesh->GetRelativeRotation() + Vector3(0.f, 0.f, 0.05f * cos(elapsedTime)));
}

void BoxGameObject::SpaceKeyDown()
{
	boxMeshComponent_->GetMesh()->SetRelativePosition(Vector3::ZeroVector);
}
