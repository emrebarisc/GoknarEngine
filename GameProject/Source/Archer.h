#pragma once

#include "Goknar/ObjectBase.h"

class Camera;
class SkeletalMesh;
class SkeletalMeshComponent;

class ArcherGameController;
class ArcherMovementComponent;
class Arrow;
class Bow;
class SosketComponent;

class Archer : public ObjectBase
{
public:
	Archer();
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

	ArcherMovementComponent* GetMovementComponent() const
	{
		return movementComponent_;
	}

	ArcherGameController* GetController() const
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

	SkeletalMesh* skeletalMesh_{ nullptr };
	SkeletalMeshComponent* skeletalMeshComponent_{ nullptr };

	Camera* thirdPersonCamera_{ nullptr };

	ArcherGameController* controller_{ nullptr };
	ArcherMovementComponent* movementComponent_{ nullptr };
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

