#pragma once

#include "Characters/BaseCharacter.h"

struct AnimationState;
struct AnimationGraph;
class CameraComponent;
class SkeletalMesh;

class SocketComponent;
class Weapon;

class GOKNAR_API AICharacter : public BaseCharacter
{
public:
	AICharacter();
	AICharacter(const Vector3& spawnProbePosition, const Vector3& moveTarget);
	virtual ~AICharacter() = default;

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	void SetSpawnProbePosition(const Vector3& spawnProbePosition);
	void SetMoveTarget(const Vector3& moveTarget);

	Vector3 GetSpawnProbePosition() const
	{
		return spawnProbePosition_;
	}

protected:

private:
	Weapon* weapon_{ nullptr };

	SkeletalMesh* skeletalMesh_{ nullptr };

	AnimationGraph* animationGraph_{ nullptr };

	Vector3 spawnProbePosition_{ Vector3::ZeroVector };
	Vector3 moveTarget_{ Vector3::ZeroVector };
};
