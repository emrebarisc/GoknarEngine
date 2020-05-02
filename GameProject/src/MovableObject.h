#pragma once
#include "Goknar\ObjectBase.h"
#include "RepairableObject.h"

class MovableObject : public ObjectBase
{
	friend class GameController;
public:
	MovableObject();
	~MovableObject();

	void BeginGame() override;
	void Tick(float deltaTime) override;
	virtual void SetParent(RepairableObject* parent) = 0;
	virtual RepairableObject* GetParent() = 0;
	void ResetFallSpeed();
	void StopMovement();
	int GetType();
	bool isMovementEnded_;
	bool canFall_;

protected:
	int type_;
	RepairableObject* parentObject_;

private:
	void MoveLeft();
	void MoveRight();
	void RollLeft();
	void RollRight();
	void IncreaseFallSpeed();
	void DecreaseFallSpeed();


	float width_;
	float height_;
	float fallSpeed_;
};

