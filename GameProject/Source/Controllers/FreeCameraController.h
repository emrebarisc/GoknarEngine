#ifndef __FREECAMERACONTROLLER_H__
#define __FREECAMERACONTROLLER_H__

#include "Goknar/Controller.h"
#include "Goknar/Delegates/Delegate.h"
#include "Goknar/Math/GoknarMath.h"

class FreeCameraObject;

class FreeCameraController : public Controller
{
public:
	FreeCameraController(FreeCameraObject* freeCameraObject);
	~FreeCameraController();

	void BeginGame() override;
	void SetupInputs() override;

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
		MoveLeft(-movementSpeed_);
	}

	void MoveLeftListener()
	{
		MoveLeft(movementSpeed_);
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
	void MoveLeft(float multiplier = 1.f);
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

	Delegate<void(double, double)> onScrollMoveDelegate_;
	Delegate<void(double, double)> onCursorMoveDelegate_;

	Vector2 previousCursorPositionForRotating_;
	Vector2 previousCursorPositionFor2DMovement_;

	float movementSpeed_;

	FreeCameraObject* freeCameraObject_;
	bool isRotatingTheCamera_;
	bool isMovingCameraIn2D_;
};

#endif