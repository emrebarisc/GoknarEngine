#pragma once

#include <memory>

#include "Goknar/Physics/Character.h"

class CharacterState;
class Controller;
class SkillContainerComponent;

class GOKNAR_API BaseCharacter : public Character
{
public:
	BaseCharacter();
	virtual ~BaseCharacter();
	virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override;

	virtual void Idle();
	virtual void RunForward();
	virtual void RunBackward();
	virtual void RunRight();
	virtual void RunLeft();
	virtual void WalkForward();
	virtual void WalkBackward();
	virtual void WalkRight();
	virtual void WalkLeft();
	virtual void Die();
	virtual void Jump();
	virtual void OnAir();
	virtual void OnGround();

	virtual void OnAttackStarted();
	virtual void OnAttackFinished();

	Controller* GetController() const
	{
		return controller_;
	}

	bool GetIsAttacking() const
	{
		return isAttacking_;
	}

protected:
	Controller* controller_{ nullptr };

private:

	bool isAttacking_{ false };
	bool blockAnimationChange_{ false };
};