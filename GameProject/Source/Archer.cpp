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
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.0125f });
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEular(Vector3{ 90.f, 0.f, 90.f }));

	SkeletalMeshInstance* skeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
	leftHandSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:LeftHand");
	leftHandSocket_->SetRelativePosition(Vector3{ 0.f, 10.f, 1.5f });
	leftHandSocket_->SetRelativeRotation(Quaternion::FromEular(Vector3{ 90.f, 90.f, 180.f }));
	leftHandSocket_->SetRelativeScaling(Vector3{ 80.f });

	bow_ = new Bow();
	bow_->AttachToSocket(leftHandSocket_);
	bow_->SetIsActive(false);

	thirdPersonCamera_ = new Camera(Vector3::ZeroVector, Vector3{ 1.f, 0.f, 0.f }, Vector3{ 0.f, 0.f, 1.f }.GetNormalized());
	thirdPersonCamera_->SetImageWidth(1600);
	thirdPersonCamera_->SetImageHeight(900);

	movementComponent_ = AddSubComponent<ArcherMovementComponent>();
	movementComponent_->SetOwnerArcher(this);

	controller_ = new ArcherGameController(this);
}

void Archer::BeginGame()
{
	GOKNAR_INFO("Archer::BeginPlay()");

	//EquipBow(true);
}

void Archer::Tick(float deltaTime)
{
	thirdPersonCamera_->SetPosition(GetWorldPosition() + Vector3(0.f, 0.f, 2.f) + thirdPersonCamera_->GetForwardVector() * -4.f * thirdPersonCameraDistance_);

	// Arrow test
	static float timer = 0.f;

	//if (1.f < timer)
	//{
	//	timer -= 1.f;

	//	Arrow* arrow = new Arrow();
	//	arrow->SetWorldPosition(GetWorldPosition() + Vector3(0.f, 0.f, 1.75f));

	//	Vector3 arrowRotationVector = (4.f * GetForwardVector() + GetUpVector()).GetNormalized();
	//	//GOKNAR_INFO("Archer forward vector: {}", GetForwardVector().ToString());
	//	//GOKNAR_INFO("Archer up vector: {}", GetUpVector().ToString());
	//	//GOKNAR_INFO("Arrow forward vector: {}", arrowRotationVector.ToString());
	//	arrow->SetWorldRotation((arrowRotationVector).GetRotationNormalized());
	//}
	//timer += deltaTime;
}

void Archer::Idle()
{
	if (isAnimationBusy_ || canShoot_)
	{
		return;
	}

	if (isBowEquiped_)
	{
		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingIdle");
	}
	else
	{
		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Idle");
	}
}

void Archer::RunForward()
{
	if (isAnimationBusy_ || canShoot_)
	{
		return;
	}

	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingRunForward");
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
		bow_ = nullptr;
	}
}

void Archer::HandleEquipBowInput()
{
	EquipBow(!isBowEquiped_);
}

void Archer::HandleDrawBowInput()
{
	if (bow_ && isBowEquiped_ && !isAiming_)
	{
		isAiming_ = true;
		isAnimationBusy_ = true;
		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDrawArrow.001", true,
			[&]()
			{
				if (isAiming_)
				{
					isAnimationBusy_ = false;
					canShoot_ = true;
					skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimIdle");
				}
			});
	}
}

void Archer::HandleLooseBowInput()
{
	if (bow_ && isBowEquiped_ && isAiming_)
	{
		isAiming_ = false;
		isLoosing_ = true;
		if (canShoot_)
		{
			isAnimationBusy_ = true;
			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimRecoil", true,
				[&]()
				{
					isAnimationBusy_ = false;
					isLoosing_ = false;
					Idle();
				});

			canShoot_ = false;
			Shoot();
		}
		else
		{
			isAnimationBusy_ = false;
			Idle();
		}
	}
	else if (!isAiming_)
	{
		Idle();
	}
}

void Archer::EquipBow(bool equip)
{
	if (bow_)
	{
		if (equip)
		{
			bow_->SetIsActive(true);
			isBowEquiped_ = true;
			isAnimationBusy_ = true;
			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingEquipBow", true,
				[&]()
				{
					isAnimationBusy_ = false;
					Idle();
				});
		}
		else
		{
			isAnimationBusy_ = true;
			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDisarmBow", true,
				[&]()
				{
					isAnimationBusy_ = false;
					bow_->SetIsActive(false);
					isBowEquiped_ = false;
					Idle();
				});
		}

	}
}

void Archer::EquipTorch(bool equip)
{

}

void Archer::Shoot() const
{
	Arrow* arrow = new Arrow();
	arrow->SetWorldPosition(GetWorldPosition() + GetForwardVector() + Vector3(0.f, 0.f, 1.75f));
	Vector3 arrowRotationVector = (4.f * GetForwardVector() + GetUpVector()).GetNormalized();
	arrow->SetWorldRotation((arrowRotationVector).GetRotationNormalized());
}
