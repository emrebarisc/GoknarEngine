#ifndef __FREECAMERACONTROLLER_H__
#define __FREECAMERACONTROLLER_H__

#include "Goknar/Controller.h"
#include "Goknar/Delegates/Delegate.h"
#include "Goknar/Math/GoknarMath.h"

class Camera;

class FreeCameraController : public Controller
{
public:
	FreeCameraController();
	~FreeCameraController();

	void BeginGame() override;
	void SetupInputs() override;

	Camera* GetFreeCamera() const
	{
		return freeCamera_;
	}

	virtual void SetIsActive(bool isActive) override;

private:
	void CursorMovement(double x, double y);
	void ScrollListener(double x, double y);

	void Yaw(float value);
	void Pitch(float value);

	void OnMouseRightClickPressed();
	void OnMouseRightClickReleased()
	{
		isRotatingTheCamera_ = false;
	}

	void DoRaycastClosestTest();
	void DoRaycastAllTest();
	void DoSweepTest();

	void OnMouseMiddleClickPressed();
	void OnMouseMiddleClickReleased()
	{
		isMovingCameraIn2D_ = false;
	}

	void MoveForwardListener()
	{
		MoveForward(movementSpeed_);
	}

	void MoveBackwardListener()
	{
		MoveForward(-movementSpeed_);
	}

	void MoveRightListener()
	{
		MoveRight(movementSpeed_);
	}

	void MoveLeftListener()
	{
		MoveRight(-movementSpeed_);
	}

	void MoveUpListener()
	{
		MoveUp(movementSpeed_);
	}

	void MoveDownListener()
	{
		MoveUp(-movementSpeed_);
	}

	void MoveForward(float multiplier = 1.f);
	void MoveRight(float multiplier = 1.f);
	void MoveUp(float multiplier = 1.f);

	void BindInputDelegates();
	void UnbindInputDelegates();

	Delegate<void()> onMouseRightClickPressedDelegate_;
	Delegate<void()> onMouseRightClickReleasedDelegate_;
	Delegate<void()> onMouseMiddleClickPressedDelegate_;
	Delegate<void()> onMouseMiddleClickReleasedDelegate_;

	Delegate<void()> moveLeftDelegate_;
	Delegate<void()> moveRightDelegate_;
	Delegate<void()> moveForwardDelegate_;
	Delegate<void()> moveBackwardDelegate_;
	Delegate<void()> moveUpDelegate_;
	Delegate<void()> moveDownDelegate_;

	Delegate<void()> doRaycastClosestTestDelegate_;
	Delegate<void()> doRaycastAllTestDelegate_;
	Delegate<void()> doSweepTestDelegate_;

	Delegate<void(double, double)> onScrollMoveDelegate_;
	Delegate<void(double, double)> onCursorMoveDelegate_;

	Camera* freeCamera_;

	Vector2 previousCursorPositionForRotating_;
	Vector2 previousCursorPositionFor2DMovement_;

	float movementSpeed_;

	bool isRotatingTheCamera_;
	bool isMovingCameraIn2D_;
};

#endif