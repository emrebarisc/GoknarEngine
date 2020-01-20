#include "pch.h"

#include "Camera.h"

#include <gl\GLU.h>

Camera::Camera(const Vector3& position, const Vector3& gaze, const Vector3& up) : 
	position_(position),
	forwardVector_(gaze),
	upVector_(up),
	leftVector_(forwardVector_.Cross(upVector_)),
	projection_(CameraProjection::Perspective)
{
	SetProjectionMatrix();
	LookAt();
}

void Camera::Init()
{
	float forwardDotUp = Vector3::Dot(forwardVector_, upVector_);

	if (-EPSILON <= forwardDotUp && forwardDotUp <= EPSILON)
	{
		leftVector_ = forwardVector_.Cross(upVector_);
		upVector_ = leftVector_.Cross(forwardVector_);
	}

	LookAt();
}

void Camera::Update()
{
	SetProjectionMatrix();
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
	position_ += leftVector_ * value;

	LookAt();
}

void Camera::Yaw(float value)
{
	forwardVector_ = forwardVector_.Rotate(upVector_ * value);
	leftVector_ = forwardVector_.Cross(upVector_);

	LookAt();
}

void Camera::Pitch(float value)
{
	forwardVector_ = forwardVector_.Rotate(leftVector_ * value);
	upVector_ = leftVector_.Cross(forwardVector_);

	LookAt();
}

void Camera::Roll(float value)
{
	leftVector_ = leftVector_.Rotate(forwardVector_ * value);
	upVector_ = leftVector_.Cross(forwardVector_);

	LookAt();
}

void Camera::RotateAbout(const Vector3& axis, float angle)
{
	Matrix rotationMatrix = Matrix::GetRotationMatrixAboutAnAxis(axis, angle);

	leftVector_ = rotationMatrix * Vector4(leftVector_, 0.f);
	upVector_ = rotationMatrix * Vector4(upVector_, 0.f);
	forwardVector_ = upVector_.Cross(leftVector_);

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