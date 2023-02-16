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
	skeletalMesh = dynamic_cast<SkeletalMesh*>(ModelLoader::LoadModel("Meshes/SkeletalMesh_Michelle.fbx"));
	skeletalMeshComponent = new SkeletalMeshComponent(this);
	skeletalMeshComponent->SetMesh(skeletalMesh);
	skeletalMeshComponent->SetRelativePosition(Vector3(0.f, 0.f, 0.f));
	skeletalMeshComponent->SetRelativeRotation(Vector3(-90.f, 0.f, 0.f));
	skeletalMeshComponent->SetRelativeScaling(Vector3(0.025f, 0.025f, 0.025f));
	dynamic_cast<SkeletalMeshInstance*>(skeletalMeshComponent->GetMeshInstance())->PlayAnimation("Armature|Armature|mixamo.com|Layer0");

	skeletalMesh2 = dynamic_cast<SkeletalMesh*>(ModelLoader::LoadModel("Meshes/SkeletalMesh_Mutant.fbx"));
	skeletalMeshComponent2 = new SkeletalMeshComponent(this);
	skeletalMeshComponent2->SetMesh(skeletalMesh2);
	skeletalMeshComponent2->SetRelativePosition(Vector3(-5.f, -5.f, 0.f));
	skeletalMeshComponent2->SetRelativeRotation(Vector3(-90.f, 0.f, 0.f));
	skeletalMeshComponent2->SetRelativeScaling(Vector3(0.025f, 0.025f, 0.025f));
	dynamic_cast<SkeletalMeshInstance*>(skeletalMeshComponent2->GetMeshInstance())->PlayAnimation("Armature|Idle2");
}

void Mutant::BeginGame()
{
}

void Mutant::Tick(float deltaTime)
{
}
