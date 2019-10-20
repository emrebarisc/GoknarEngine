#include "pch.h"

#include "Camera.h"

#include <gl\GLU.h>

Camera::Camera(const Vector3& position, const Vector3& gaze, const Vector3& up) : 
	position_(position),
	forwardVector_(gaze),
	upVector_(up),
	rightVector_(forwardVector_.Cross(upVector_)),
	type_(CameraType::Perspective)
{
	SetProjectionMatrix();
	LookAt();
}

void Camera::InitCamera()
{
	float forwardDotUp = Vector3::Dot(forwardVector_, upVector_);

	if (-EPSILON <= forwardDotUp && forwardDotUp <= EPSILON)
	{
		rightVector_ = forwardVector_.Cross(upVector_);
		upVector_ = rightVector_.Cross(forwardVector_);
	}

	LookAt();
}

void Camera::MoveForward(float value)
{
	position_ += forwardVector_ * value;

	LookAt();
}

void Camera::MoveUpward(float value)
{
	position_ += upVector_ * value;

	LookAt();
}

void Camera::MoveRight(float value)
{
	position_ += rightVector_ * value;

	LookAt();
}

void Camera::Yaw(float value)
{
	forwardVector_ = forwardVector_.Rotate(upVector_ * value);
	rightVector_ = forwardVector_.Cross(upVector_);

	LookAt();
}

void Camera::Pitch(float value)
{
	forwardVector_ = forwardVector_.Rotate(rightVector_ * value);
	upVector_ = rightVector_.Cross(forwardVector_);

	LookAt();
}

void Camera::Roll(float value)
{
	rightVector_ = rightVector_.Rotate(forwardVector_ * value);
	upVector_ = rightVector_.Cross(forwardVector_);

	LookAt();
}

void Camera::RotateAbout(const Vector3& axis, float angle)
{
	Matrix rotationMatrix = Matrix::GetRotationMatrixAboutAnAxis(axis, angle);

	rightVector_ = rotationMatrix * Vector4(rightVector_, 0.f);
	upVector_ = rotationMatrix * Vector4(upVector_, 0.f);
	forwardVector_ = upVector_.Cross(rightVector_);

	LookAt();
}

void Camera::LookAt()
{
	Vector3 lookAtPos = position_ + forwardVector_ * nearDistance_;

	Math::LookAt(viewingMatrix_, 
				 position_,
				 lookAtPos,
				 upVector_);
}