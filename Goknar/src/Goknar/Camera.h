#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Goknar/Core.h"
#include "Goknar/Matrix.h"

class GOKNAR_API Camera
{
public:
	Camera() : 
		viewingMatrix_(Matrix::IdentityMatrix),
		position_(Vector3::ZeroVector),
		forwardVector_(Vector3::ForwardVector),
		rightVector_(Vector3::RightVector),
		upVector_(Vector3::UpVector),
		nearPlane_(0.1f),
		nearDistance_(0.1f),
		farDistance_(100.f),
		imageWidth_(1024),
		imageHeight_(768)
	{
	}

	Camera(const Vector3& position, const Vector3& forward, const Vector3& up);

	Camera(const Camera* rhs)
	{
		if (rhs != this)
		{
			viewingMatrix_ = rhs->viewingMatrix_;
			position_ = rhs->position_;
			forwardVector_ = rhs->forwardVector_;
			upVector_ = rhs->upVector_;
			rightVector_ = rhs->rightVector_;
			nearPlane_ = rhs->nearPlane_;
			nearDistance_ = rhs->nearDistance_;
			farDistance_ = rhs->farDistance_;
			imageWidth_ = rhs->imageWidth_;
			imageHeight_ = rhs->imageHeight_;
		}
	}

	void InitCamera();

	void MoveForward(float value);
	void LookRight(float value);
	void LookUp(float value);

	const Matrix& GetViewingMatrix() const
	{
		return viewingMatrix_;
	}

	void SetPosition(const Vector3& position)
	{
		position_ = position;
	}

	Vector3 GetPosition() const
	{
		return position_;
	}

	void SetForwardVector(const Vector3& forwardVector)
	{
		forwardVector_ = forwardVector;
	}

	const Vector3& GetForwardVector() const
	{
		return forwardVector_;
	}

	void SetUpVector(const Vector3& upVector)
	{
		upVector_ = upVector;
	}

	const Vector3& GetUpVector() const
	{
		return upVector_;
	}

	void SetRightVector(const Vector3& rightVector)
	{
		rightVector_ = rightVector;
	}

	const Vector3& GetRightVector() const
	{
		return rightVector_;
	}

	void SetNearPlane(const Vector3& nearPlane)
	{
		nearPlane_ = nearPlane;
	}

	const Vector4& GetNearPlane() const
	{
		return nearPlane_;
	}

protected:

private:
	void LookAt();

	Matrix viewingMatrix_;

	Vector4 nearPlane_;

	Vector3 position_;
	Vector3 forwardVector_;
	Vector3 upVector_;
	Vector3 rightVector_;

	float nearDistance_;
	float farDistance_;
	int imageWidth_, imageHeight_;
};

#endif