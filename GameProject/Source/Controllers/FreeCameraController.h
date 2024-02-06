#ifndef __FREECAMERACONTROLLER_H__
#define __FREECAMERACONTROLLER_H__

#include <Goknar/Components/Component.h>
#include "Goknar/Math/GoknarMath.h"

class Camera;

class FreeCameraController : public Component
{
public:
	FreeCameraController(Component* parent);
	~FreeCameraController();

	void BeginGame() override;

	Camera* GetFreeCamera() const
	{
		return freeCamera_;
	}

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

	Camera* freeCamera_;

	Vector2 previousCursorPositionForRotating_;
	Vector2 previousCursorPositionFor2DMovement_;

	float movementSpeed_;

	bool isRotatingTheCamera_;
	bool isMovingCameraIn2D_;
};

#endif