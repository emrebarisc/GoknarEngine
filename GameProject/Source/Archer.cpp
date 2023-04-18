#include "Archer.h"

#include "IO/ModelLoader.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SocketComponent.h"
#include "Components/ProjectileMovementComponent.h"
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

	rightHandSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:RightHand");
	rightHandSocket_->SetRelativePosition(Vector3{ 0.f, 10.f, 0.f });
	rightHandSocket_->SetRelativeRotation(Quaternion::FromEular(Vector3{ 0.f, -30.f, 160.f }));
	rightHandSocket_->SetRelativeScaling(Vector3{ 80.f });

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

	EquipBow(true);
}

void Archer::Tick(float deltaTime)
{
	thirdPersonCamera_->SetPosition(GetWorldPosition() + Vector3(0.f, 0.f, 2.f) + thirdPersonCamera_->GetForwardVector() * -4.f * thirdPersonCameraDistance_);
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

		KeyframeData keyframeData;
		keyframeData.AddCallbackToKeyframe(8,
			[&]()
			{
				loadedArrow_ = new Arrow();
				loadedArrow_->AttachToSocket(rightHandSocket_);
				//loadedArrow_->SetWorldPosition(GetWorldPosition() + GetForwardVector() + Vector3(0.f, 0.f, 1.75f));
				//Vector3 arrowRotationVector = (4.f * GetForwardVector() + GetUpVector()).GetNormalized();
				//loadedArrow_->SetWorldRotation((arrowRotationVector).GetRotationNormalized());
			});

		keyframeData.AddCallbackToKeyframe(20,
			[&]()
			{
			});

		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDrawArrow.001",
			PlayLoopData{
				true,
				[&]()
				{
					if (isAiming_)
					{
						isAnimationBusy_ = false;
						canShoot_ = true;
						skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimIdle");
					}
				} 
			}, keyframeData);
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

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimRecoil",
				PlayLoopData{
					true,
					[&]()
					{
						isAnimationBusy_ = false;
						isLoosing_ = false;
						Idle();
					}
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
			isAnimationBusy_ = true;
			
			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(8, 
				[&]() 
				{
					bow_->SetIsActive(true);
				});

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingEquipBow",
				{
					false,
					[&]()
					{
						isBowEquiped_ = true;
						isAnimationBusy_ = false;
						Idle();
					}
				}, keyframeData);
		}
		else
		{
			isAnimationBusy_ = true;

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(
				14,
				[&]()
				{
					bow_->SetIsActive(false);
				});

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDisarmBow",
				PlayLoopData{
					true,
					[&]()
					{
						isAnimationBusy_ = false;
						isBowEquiped_ = false;
						Idle();
					}
				}, keyframeData);
		}

	}
}

void Archer::EquipTorch(bool equip)
{

}

void Archer::Shoot() const
{
	if (loadedArrow_)
	{
		loadedArrow_->RemoveFromSocket(rightHandSocket_);
		loadedArrow_->GetMovementComponent()->SetIsActive(true);
	}
}
