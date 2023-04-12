#include "Arrow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Components/ProjectileMovementComponent.h"

Arrow::Arrow() : StaticMeshObject()
{
	StaticMesh* arrowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Arrow.fbx");
	staticMeshComponent->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
}

void Arrow::BeginGame()
{
	ObjectBase::BeginGame();

	GOKNAR_INFO("Arrow::BeginGame");
}
