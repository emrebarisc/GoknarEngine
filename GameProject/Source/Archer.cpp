#include "Archer.h"

#include "IO/ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Materials/MaterialBase.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "ArcherGameController.h"
#include "Components/ArcherMovementComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Objects/Arrow.h"
#include "Objects/AxisObject.h"
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
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));

	SkeletalMeshInstance* skeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
	leftHandSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:LeftHand");
	leftHandSocket_->SetRelativePosition(Vector3{ 0.f, 10.f, 1.5f });
	leftHandSocket_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 90.f, 180.f }));
	leftHandSocket_->SetRelativeScaling(Vector3{ 80.f });

	rightHandSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:RightHand");
	rightHandSocket_->SetRelativePosition(Vector3{ 0.f, 10.f, 0.f });
	rightHandSocket_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 0.f, -13.5f, 90.f }));
	rightHandSocket_->SetRelativeScaling(Vector3{ 80.f });

	bowStringSocket_ = skeletalMeshInstance->AddSocketToBone("mixamorig:RightHandMiddle3");
	bowStringSocket_->SetRelativeScaling(Vector3{ 80.f });
	bowStringSocket_->SetRelativePosition(Vector3{ 14.f, 0.f, 0.f });

	bow_ = new Bow();
	bow_->AttachToSocket(leftHandSocket_);
	bow_->SetIsActive(false);

	thirdPersonCamera_ = new Camera(Vector3::ZeroVector, Vector3{ 1.f, 0.f, 0.f }, Vector3{ 0.f, 0.f, 1.f }.GetNormalized());
	//thirdPersonCamera_->SetImageWidth(1920);
	//thirdPersonCamera_->SetImageHeight(1080);
	thirdPersonCamera_->SetImageWidth(1600);
	thirdPersonCamera_->SetImageHeight(900);
	thirdPersonCamera_->SetNearDistance(1.f);
	thirdPersonCamera_->SetFarDistance(1000.f);
	thirdPersonCamera_->SetFOV(45.f);

	movementComponent_ = AddSubComponent<ArcherMovementComponent>();
	movementComponent_->SetOwnerArcher(this);

	controller_ = new ArcherGameController(this);
}

void AddAxisToAllBones(SkeletalMeshInstance* skeletalMeshInstance, Bone* bone)
{
	AxisObject* axisObject = new AxisObject();
	SocketComponent* socket = skeletalMeshInstance->AddSocketToBone(bone->name);
	axisObject->AttachToSocket(socket);

	for (auto child : bone->children)
	{
		AddAxisToAllBones(skeletalMeshInstance, child);
	}
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
	if (bow_ && !isAiming_)
	{
		bow_->RemoveFromSocket(leftHandSocket_);
		GOKNAR_INFO("Bow forward vector: {}", bow_->GetForwardVector().ToString());
		bow_ = nullptr;

		isBowEquiped_ = false;
		Idle();
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
		keyframeData.AddCallbackToKeyframe(9,
			[&]()
			{
				loadedArrow_ = new Arrow();
				loadedArrow_->AttachToSocket(rightHandSocket_);
			});

		keyframeData.AddCallbackToKeyframe(18,
			[&]()
			{
				bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", &bowStringSocket_->GetComponentToWorldTransformationMatrix());
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

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(5,
				[&]()
				{
					Shoot();
					bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", nullptr);
				});

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimRecoil",
				PlayLoopData{
					true,
					[&]()
					{
						isAnimationBusy_ = false;
						isLoosing_ = false;
						Idle();
					}
				}, keyframeData);

			canShoot_ = false;
		}
		else
		{
			isAnimationBusy_ = false;

			if (loadedArrow_)
			{
				loadedArrow_->Destroy();
				loadedArrow_ = nullptr;
			}
			bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", nullptr);
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

void Archer::Shoot()
{
	if (loadedArrow_)
	{
		loadedArrow_->RemoveFromSocket(rightHandSocket_);
		loadedArrow_->Shoot();
		loadedArrow_ = nullptr;
	}
}
