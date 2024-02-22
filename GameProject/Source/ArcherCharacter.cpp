#include "ArcherCharacter.h"

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

#include "ArcherCharacterController.h"
#include "Components/ArcherCharacterMovementComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Objects/Arrow.h"
#include "Objects/AxisObject.h"
#include "Objects/Bow.h"

ArcherCharacter::ArcherCharacter() : Character()
{
	SetIsTickable(true);

	SetTag("Archer");

	movementComponent_ = AddSubComponent<ArcherCharacterMovementComponent>();

	capsuleCollisionComponent_->SetRadius(0.3f);
	capsuleCollisionComponent_->SetHeight(1.8f);
	capsuleCollisionComponent_->SetRelativePosition(Vector3{0.f, 0.f, -0.3f});

	capsuleCollisionComponent_->OnOverlapBegin = Delegate<OverlapBeginAlias>::create<ArcherCharacter, &ArcherCharacter::OnOverlapBegin>(this);
	//capsuleCollisionComponent_->OnOverlapContinue = Delegate<OverlapContinueAlias>::create<ArcherCharacter, &ArcherCharacter::OnOverlapContinue>(this);
	capsuleCollisionComponent_->OnOverlapEnd = Delegate<OverlapEndAlias>::create<ArcherCharacter, &ArcherCharacter::OnOverlapEnd>(this);

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Akai.fbx");
	skeletalMesh_->GetMaterial()->SetSpecularReflectance(Vector3{0.f});

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
	thirdPersonCamera_->SetImageWidth(1920);
	thirdPersonCamera_->SetImageHeight(1000);
	thirdPersonCamera_->SetNearDistance(1.f);
	thirdPersonCamera_->SetFarDistance(1000.f);
	thirdPersonCamera_->SetFOV(45.f);

	controller_ = new ArcherCharacterController(this);
}

void ArcherCharacter::BeginGame()
{
	EquipBow(true);
}

void ArcherCharacter::Tick(float deltaTime)
{
	thirdPersonCamera_->SetPosition(GetWorldPosition() + Vector3(0.f, 0.f, 2.f) + thirdPersonCamera_->GetForwardVector() * -4.f * thirdPersonCameraDistance_);
}

void ArcherCharacter::OnOverlapBegin(PhysicsObject* otherObject, class CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3&  hitNormal)
{
	GOKNAR_INFO("OnOverlapBegin with {}", otherObject->GetName());
}

void ArcherCharacter::OnOverlapContinue(PhysicsObject* otherObject, class CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3&  hitNormal)
{
	GOKNAR_INFO("OnOverlapContinue with {}", otherObject->GetName());
}

void ArcherCharacter::OnOverlapEnd(PhysicsObject* otherObject, class CollisionComponent* otherComponent)
{
	GOKNAR_INFO("OnOverlapEnd with {}", otherObject->GetName());
}

void ArcherCharacter::Idle()
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

void ArcherCharacter::RunForward()
{
	if (isAnimationBusy_ || canShoot_)
	{
		return;
	}

	skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingRunForward");
}

void ArcherCharacter::RunBackward()
{
}

void ArcherCharacter::RunRight()
{
}

void ArcherCharacter::RunLeft()
{
}

void ArcherCharacter::HandleDropBowInput()
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

void ArcherCharacter::HandleEquipBowInput()
{
	EquipBow(!isBowEquiped_);
}

void ArcherCharacter::HandleDrawBowInput()
{
	if (bow_ && isBowEquiped_ && !isAiming_)
	{
		isAiming_ = true;
		isAnimationBusy_ = true;

		KeyframeData keyframeData;
		keyframeData.AddCallbackToKeyframe(9, Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnCreateArrow>(this));
		keyframeData.AddCallbackToKeyframe(18, Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnAttachBowStringToHand>(this));

		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDrawArrow.001",
			PlayLoopData
			{
				true,
				Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnAimingIdle>(this)
			}, keyframeData);
	}
}

void ArcherCharacter::HandleLooseBowInput()
{
	if (bow_ && isBowEquiped_ && isAiming_)
	{
		isAiming_ = false;
		isLoosing_ = true;
		if (canShoot_)
		{
			isAnimationBusy_ = true;

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(3, Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnShoot>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimRecoil",
				PlayLoopData{
					true,
					Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnFinishedLoosing>(this)
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

void ArcherCharacter::EquipBow(bool equip)
{
	if (bow_)
	{
		if (equip)
		{
			isAnimationBusy_ = true;
			
			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(8, Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnBowIsHandled>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingEquipBow",
				{
					false,
					Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnFinishedEquipingBow>(this)
				}, keyframeData);
		}
		else
		{
			isAnimationBusy_ = true;

			KeyframeData keyframeData;
			keyframeData.AddCallbackToKeyframe(14, Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnBowIsUnhandled>(this));

			skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingDisarmBow",
				PlayLoopData{
					true,
					Delegate<void()>::create<ArcherCharacter, &ArcherCharacter::OnBowIsDisarmed>(this)
				}, keyframeData);
		}

	}
}

void ArcherCharacter::EquipTorch(bool equip)
{

}

void ArcherCharacter::Shoot()
{
	if (loadedArrow_)
	{
		loadedArrow_->RemoveFromSocket(rightHandSocket_);
		loadedArrow_->Shoot();
		loadedArrow_ = nullptr;
	}
}

void ArcherCharacter::OnCreateArrow()
{
	loadedArrow_ = new Arrow();
	loadedArrow_->AttachToSocket(rightHandSocket_);
}

void ArcherCharacter::OnAttachBowStringToHand()
{
	bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", &bowStringSocket_->GetComponentToWorldTransformationMatrix());
}

void ArcherCharacter::OnAimingIdle()
{
	if (isAiming_)
	{
		isAnimationBusy_ = false;
		canShoot_ = true;
		skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("StandingAimIdle");
	}
}

void ArcherCharacter::OnShoot()
{
	Shoot();
	bow_->GetSkeletalMesh()->GetMeshInstance()->AttachBoneToMatrixPointer("BowString", nullptr);
}

void ArcherCharacter::OnFinishedLoosing()
{
	isAnimationBusy_ = false;
	isLoosing_ = false;
	Idle();
}

void ArcherCharacter::OnBowIsHandled()
{
	bow_->SetIsActive(true);
}

void ArcherCharacter::OnFinishedEquipingBow()
{
	isBowEquiped_ = true;
	isAnimationBusy_ = false;
	Idle();
}

void ArcherCharacter::OnBowIsUnhandled()
{
	bow_->SetIsActive(false);
}

void ArcherCharacter::OnBowIsDisarmed()
{
	isAnimationBusy_ = false;
	isBowEquiped_ = false;
	Idle();
}