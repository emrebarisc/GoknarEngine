#include "Mutant.h"

#include "IO/ModelLoader.h"

#include "Application.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"

#include "Components/SkeletalMeshComponent.h"
#include "Model/SkeletalMesh.h"
#include "Model/SkeletalMeshInstance.h"

Mutant::Mutant() : ObjectBase()
{
	skeletalMesh = dynamic_cast<SkeletalMesh*>(ModelLoader::LoadModel("Meshes/SkeletalMesh_Mutant.fbx"));
	skeletalMesh->SetMaterial(engine->GetApplication()->GetMainScene()->GetMaterial(0));


	skeletalMeshComponent = new SkeletalMeshComponent(this);
	skeletalMeshComponent->SetMesh(skeletalMesh);
	skeletalMeshComponent->SetRelativePosition(Vector3(-5.f, -5.f, 0.1f));
	skeletalMeshComponent->SetRelativeRotation(Vector3(-90.f, 0.f, 0.f));
	skeletalMeshComponent->SetRelativeScaling(Vector3(0.025f, 0.025f, 0.025f));
}

void Mutant::BeginGame()
{
}

void Mutant::Tick(float deltaTime)
{
}
