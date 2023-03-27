#ifndef __EDITORCAMERACONTROLLER_H__
#define __EDITORCAMERACONTROLLER_H__

#include <Goknar/ObjectBase.h>

class Camera;

class EditorCameraController : public ObjectBase
{
public:
	EditorCameraController();
	~EditorCameraController();

	void BeginGame() override;
	void Tick(float deltaTime) override;

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

	Camera* activeCamera_;

	Vector2 previousCursorPositionForRotating_;
	Vector2 previousCursorPositionFor2DMovement_;

	float movementSpeed_;

	bool isRotatingTheCamera_;
	bool isMovingCameraIn2D_;
};

#endif