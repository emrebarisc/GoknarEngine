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
	leftHandSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:LeftHand");
	leftHandSocket_->SetRelativePosition(Vector3(0.f, 10.f, 1.f));
	leftHandSocket_->SetRelativeRotation(Vector3(10.f, 0.f, 0.f));
	leftHandSocket_->SetRelativeScaling(Vector3(80.f));

	bow_ = new Bow();
	bow_->AttachToSocket(leftHandSocket_);
	bow_->SetIsActive(false);

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

void Archer::Idle()
{
	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Armature|Idle");
}

void Archer::RunForward()
{
	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Armature|StandingRunForward");
}

void Archer::RunBackward()
{
}

void Archer::RunRight()
{
}

void Archer::RunLeft()
{
}

void Archer::HandleDropBowInput()
{
	if (bow_)
	{
		bow_->RemoveFromSocket(leftHandSocket_);
	}
}

void Archer::HandleEquipBowInput()
{
	EquipBow(!isBowEquiped_);
}

void Archer::EquipBow(bool equip)
{
	bow_->SetIsActive(equip);
	isBowEquiped_ = equip;
}

void Archer::EquipTorch(bool equip)
{

}
