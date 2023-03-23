#include "Archer.h"

#include "IO/ModelLoader.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"

#include "Components/SkeletalMeshComponent.h"
#include "Managers/CameraManager.h"
#include "Managers/ResourceManager.h"
#include "Model/SkeletalMesh.h"
#include "Model/SkeletalMeshInstance.h"

Archer::Archer() : ObjectBase()
{
	SetTickable(true);

	skeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Akai.fbx");
	skeletalMeshComponent = new SkeletalMeshComponent(this);
	skeletalMeshComponent->SetMesh(skeletalMesh);
	skeletalMeshComponent->SetRelativePosition(Vector3::ZeroVector);
	dynamic_cast<SkeletalMeshInstance*>(skeletalMeshComponent->GetMeshInstance())->PlayAnimation("Armature|Idle");

	followCamera = new Camera(Vector3::ZeroVector, Vector3(1.f, 0.f, -1.f).GetNormalized(), Vector3(1.f, 0.f, 1.f).GetNormalized());
}

void Archer::BeginGame()
{
	followCamera->Init();
	engine->GetCameraManager()->AddCamera(followCamera);
}

void Archer::Tick(float deltaTime)
{
	followCamera->SetPosition(GetWorldPosition() + Vector3(-2.f, 0.f, 2.5f));
}
