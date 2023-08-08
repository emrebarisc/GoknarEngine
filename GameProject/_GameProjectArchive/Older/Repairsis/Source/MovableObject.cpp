#include "MovableObject.h"
#include "MovableObjectFactory.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Scene.h"

#include "GridManager.h"

MovableObject::MovableObject() : isMovementEnded_(false), canFall_(true)
{
	SetTickable(true);
	fallSpeed_ = 2.f;
}

MovableObject::~MovableObject()
{
}

void MovableObject::BeginGame()
{
}

void MovableObject::Tick(float deltaTime)
{
	if (!canFall_)	return;
	SetWorldPosition(GetWorldPosition() + Vector3(0.f, -1.f, 0.f) * fallSpeed_ * deltaTime);
	GridManager::GetInstance()->NotifyGridManager(this, GetWorldPosition());
}

void MovableObject::ResetFallSpeed()
{
	fallSpeed_ = 2.f;
	canFall_ = true;
}

void MovableObject::StopMovement()
{
	fallSpeed_ = 0.f;
	isMovementEnded_ = true;
	canFall_ = false;
	Vector3 currentPosition = GetWorldPosition();

	SetWorldPosition(Vector3(currentPosition.x, ceil(currentPosition.y), currentPosition.z));
}

void MovableObject::RollLeft()
{
	//SetWorldRotation(GetWorldRotation() + Vector3(0.f, 0.f, DEGREE_TO_RADIAN(-90.f)));
}

void MovableObject::RollRight()
{
	//SetWorldRotation(GetWorldRotation() + Vector3(0.f, 0.f, DEGREE_TO_RADIAN(90.f)));
}

void MovableObject::MoveLeft()
{
	Vector3 newPosition = GetWorldPosition() + Vector3(-1.f, 0.f, 0.f);
	if (GridManager::GetInstance()->IsGridEmpty(newPosition))
	{
		SetWorldPosition(newPosition);
	}
}

void MovableObject::MoveRight()
{
	Vector3 newPosition = GetWorldPosition() + Vector3(1.f, 0.f, 0.f);
	if (GridManager::GetInstance()->IsGridEmpty(newPosition))
	{
		SetWorldPosition(newPosition);
	}
}

void MovableObject::IncreaseFallSpeed()
{
	fallSpeed_ = 8.f;
}

void MovableObject::DecreaseFallSpeed()
{
	fallSpeed_ = 2.f;
}

int MovableObject::GetType()
{
	return type_;
}
