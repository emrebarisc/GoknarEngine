#include "PhysicsArcher.h"

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
#include "Goknar/Materials/Material.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"

#include "PhysicsArcherGameController.h"
#include "Components/PhysicsArcherMovementComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Objects/Arrow.h"
#include "Objects/AxisObject.h"
#include "Objects/Bow.h"

PhysicsArcher::PhysicsArcher() : 
	RigidBody()
{
	SetIsTickable(true);

	SetAngularFactor(Vector3(0.f, 0.f, 0.f));
	SetMass(60.f);

	capsuleCollisionComponent_ = AddSubComponent<CapsuleCollisionComponent>();
	capsuleCollisionComponent_->SetRadius(0.3f);
	capsuleCollisionComponent_->SetHeight(1.8f);
	capsuleCollisionComponent_->SetRelativePosition(Vector3{0.f, 0.f, -0.3f});
	SetRootComponent(capsuleCollisionComponent_);

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Akai.fbx");
	skeletalMesh_->GetMaterial()->SetSpecularReflectance(Vector3{0.f});

	skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetRelativePosition(Vector3::ZeroVector);
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.0125f });
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
	skeletalMeshComponent_->SetParent(capsuleCollisionComponent_);

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

	movementComponent_ = AddSubComponent<PhysicsArcherMovementComponent>();
	movementComponent_->SetOwnerPhysicsArcher(this);

	controller_ = new PhysicsArcherGameController(this);
}

void PhysicsArcher::BeginGame()
{
	GOKNAR_INFO("PhysicsArcher::BeginPlay()");

	EquipBow(true);
}

void PhysicsArcher::Tick(float deltaTime)
{
	thirdPersonCamera_->SetPosition(GetWorldPosition() + Vector3(0.f, 0.f, 2.f) + thirdPersonCamera_->GetForwardVector() * -4.f * thirdPersonCameraDistance_);
}

void PhysicsArcher::Idle()
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

void PhysicsArcher::RunForward()
{
	if (isAnimationBusy_ || canShoot_)
	{
		return;
	}

	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingRunForward");
}

void PhysicsArcher::RunBackward()
{
}

void PhysicsArcher::RunRight()
{
}

void PhysicsArcher::RunLeft()
{
}

void PhysicsArcher::HandleDropBowInput()
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

void PhysicsArcher::HandleEquipBowInput()
{
	EquipBow(!isBowEquiped_);
}

void PhysicsArcher::HandleDrawBowInput()
{
	if (bow_ && isBowEquiped_ && !isAiming_)
	{
		isAiming_ = true;
		isAnimationBusy_ = true;

		KeyframeData keyframeData;
		keyframeData.AddCallbackToKeyframe(9, Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnCreateArrow>(this));
		keyframeData.AddCallbackToKeyframe(18, Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnAttachBowStringToHand>(this));

		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDrawArrow.001",
			PlayLoopData
			{
				true,
				Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnAimingIdle>(this)
			}, keyframeData);
	}
}

void PhysicsArcher::HandleLooseBowInput()
{
	if (bow_ && isBowEquiped_ && isAiming_)
	{
		isAiming_ = false;
		isLoosing_ = true;
		if (canShoot_)
		{
			isAnimationBusy_ = true;

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(3, Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnShoot>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimRecoil",
				PlayLoopData{
					true,
					Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnFinishedLoosing>(this)
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

void PhysicsArcher::EquipBow(bool equip)
{
	if (bow_)
	{
		if (equip)
		{
			isAnimationBusy_ = true;
			
			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(8, Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnBowIsHandled>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingEquipBow",
				{
					false,
					Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnFinishedEquipingBow>(this)
				}, keyframeData);
		}
		else
		{
			isAnimationBusy_ = true;

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(14, Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnBowIsUnhandled>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDisarmBow",
				PlayLoopData{
					true,
					Delegate<void()>::create<PhysicsArcher, &PhysicsArcher::OnBowIsDisarmed>(this)
				}, keyframeData);
		}

	}
}

void PhysicsArcher::EquipTorch(bool equip)
{

}

void PhysicsArcher::Shoot()
{
	if (loadedArrow_)
	{
		loadedArrow_->RemoveFromSocket(rightHandSocket_);
		loadedArrow_->Shoot();
		loadedArrow_ = nullptr;
	}
}

void PhysicsArcher::OnCreateArrow()
{
	loadedArrow_ = new Arrow();
	loadedArrow_->AttachToSocket(rightHandSocket_);
}

void PhysicsArcher::OnAttachBowStringToHand()
{
	bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", &bowStringSocket_->GetComponentToWorldTransformationMatrix());
}

void PhysicsArcher::OnAimingIdle()
{
	if (isAiming_)
	{
		isAnimationBusy_ = false;
		canShoot_ = true;
		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimIdle");
	}
}

void PhysicsArcher::OnShoot()
{
	Shoot();
	bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", nullptr);
}

void PhysicsArcher::OnFinishedLoosing()
{
	isAnimationBusy_ = false;
	isLoosing_ = false;
	Idle();
}

void PhysicsArcher::OnBowIsHandled()
{
	bow_->SetIsActive(true);
}

void PhysicsArcher::OnFinishedEquipingBow()
{
	isBowEquiped_ = true;
	isAnimationBusy_ = false;
	Idle();
}

void PhysicsArcher::OnBowIsUnhandled()
{
	bow_->SetIsActive(false);
}

void PhysicsArcher::OnBowIsDisarmed()
{
	isAnimationBusy_ = false;
	isBowEquiped_ = false;
	Idle();
}