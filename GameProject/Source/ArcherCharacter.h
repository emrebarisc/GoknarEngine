#pragma once

#include "Goknar/Physics/Character.h"

class Camera;
class SkeletalMesh;

class ArcherCharacterController;
class Arrow;
class Bow;
class SocketComponent;

class ArcherCharacter : public Character
{
public:
	ArcherCharacter();
	virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override;

	SkeletalMesh* GetSkeletalMesh() const
	{
		return skeletalMesh_;
	}

	SkeletalMeshComponent* GetSkeletalMeshComponent() const
	{
		return skeletalMeshComponent_;
	}

	ArcherCharacterController* GetController() const
	{
		return controller_;
	}

	Camera* GetThirdPersonCamera() const
	{
		return thirdPersonCamera_;
	}

	void IncreaseThirdPersonCameraDistance()
	{
		if (thirdPersonCameraDistance_ < 5.f)
		{
			thirdPersonCameraDistance_ += 0.25f;
		}
	}
	
	void DecreaseThirdPersonCameraDistance()
	{
		if (0.25f < thirdPersonCameraDistance_)
		{
			thirdPersonCameraDistance_ -= 0.25f;
		}
	}

	void Idle();
	void RunForward();
	void RunBackward();
	void RunRight();
	void RunLeft();

	void HandleDropBowInput();
	void HandleEquipBowInput();
	void HandleDrawBowInput();
	void HandleLooseBowInput();

private:
	void EquipBow(bool equip);
	void EquipTorch(bool equip);

	void Shoot();

	void OnOverlapBegin(PhysicsObject* otherObject, class CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3&  hitNormal);
	void OnOverlapContinue(PhysicsObject* otherObject, class CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3&  hitNormal);
	void OnOverlapEnd(PhysicsObject* otherObject, class CollisionComponent* otherComponent);

	// ----- Delegate Callbacks -----
	void OnCreateArrow();
	void OnAttachBowStringToHand();
	void OnAimingIdle();
	void OnShoot();
	void OnFinishedLoosing();
	void OnBowIsHandled();
	void OnFinishedEquipingBow();
	void OnBowIsUnhandled();
	void OnBowIsDisarmed();
	// ------------------------------

	SkeletalMesh* skeletalMesh_{ nullptr };

	Camera* thirdPersonCamera_{ nullptr };

	ArcherCharacterController* controller_{ nullptr };
	
	Arrow* loadedArrow_{ nullptr };
	Bow* bow_{ nullptr };

	SocketComponent* leftHandSocket_{ nullptr };
	SocketComponent* rightHandSocket_{ nullptr };
	SocketComponent* bowStringSocket_{ nullptr };

	float thirdPersonCameraDistance_{ 1.f };

	bool isBowEquiped_{ false };
	bool isTorchEquiped_{ false };

	bool canShoot_{ false };
	bool isAiming_{ false };
	bool isLoosing_{ false };
	bool isAnimationBusy_{ false };
};

