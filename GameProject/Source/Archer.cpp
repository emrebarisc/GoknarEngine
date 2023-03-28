#include "Archer.h"

#include "IO/ModelLoader.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SocketComponent.h"
#include "Managers/CameraManager.h"
#include "Managers/ResourceManager.h"
#include "Model/SkeletalMesh.h"
#include "Model/SkeletalMeshInstance.h"

#include "ArcherGameController.h"
#include "Components/ArcherMovementComponent.h"
#include "Objects/Arrow.h"
#include "Objects/Bow.h"

Archer::Archer() : 
	ObjectBase()
{
	SetIsTickable(true);

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Akai.fbx");
	skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetRelativePosition(Vector3::ZeroVector);
	skeletalMeshComponent_->SetRelativeRotation(Vector3(-90.f, -90.f, 0.f));
	skeletalMeshComponent_->SetRelativeScaling(Vector3(0.0125f));

	SkeletalMeshInstance* skeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
	SocketComponent* bowHandSocket = skeletalMeshInstance->AddSocketToBone("mixamorig:LeftHand");

	Bow* bow = new Bow();
	bow->AttachToSocket(bowHandSocket);

	thirdPersonCamera_ = new Camera(Vector3::ZeroVector, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f).GetNormalized());

	movementComponent_ = AddSubComponent<ArcherMovementComponent>();
	movementComponent_->SetOwnerArcher(this);

	controller_ = new ArcherGameController(this);
}

void Archer::BeginGame()
{
	GOKNAR_INFO("Archer::BeginPlay()");
}

void Archer::Tick(float deltaTime)
{
	thirdPersonCamera_->SetPosition(GetWorldPosition() + Vector3(0.f, 0.f, 2.f) + thirdPersonCamera_->GetForwardVector() * -4.f * thirdPersonCameraDistance_);

	// Arrow test
	//static float timer = 0.f;

	//if (1.f < timer)
	//{
	//	timer -= 1.f;

	//	Arrow* arrow = new Arrow();
	//	arrow->SetWorldPosition(GetWorldPosition());
	//	arrow->SetWorldRotation(GetForwardVector().GetRotation());
	//}
	//timer += deltaTime;
}