#ifndef __CAMERACONTROLLER_H__
#define __CAMERACONTROLLER_H__

#include <Goknar/ObjectBase.h>

class Camera;
class MeshComponent;

class CameraController : public ObjectBase
{
public:
	CameraController();
	~CameraController();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	void CursorMovement(double x, double y);

	void Yaw(float value);
	void Pitch(float value);

	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();

	Camera* activeCamera_;

	Vector2 previousCursorPosition_;

	float movementSpeed_;
};

#endif