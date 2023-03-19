#include "Archer.h"

#include "IO/ModelLoader.h"

#include "Application.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"

#include "Components/SkeletalMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Model/SkeletalMesh.h"
#include "Model/SkeletalMeshInstance.h"

Archer::Archer() : ObjectBase()
{
	//skeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Archer.fbx");
	//skeletalMeshComponent = new SkeletalMeshComponent(this);
	//skeletalMeshComponent->SetMesh(skeletalMesh);
	//skeletalMeshComponent->SetRelativePosition(Vector3::ZeroVector);
	//dynamic_cast<SkeletalMeshInstance*>(skeletalMeshComponent->GetMeshInstance())->PlayAnimation("Armature|Idle");
}

void Archer::BeginGame()
{
}

void Archer::Tick(float deltaTime)
{
}
