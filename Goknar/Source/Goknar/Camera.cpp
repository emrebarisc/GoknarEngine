#include "pch.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

#ifdef GOKNAR_PLATFORM_WINDOWS
#include <GL/GLU.h>
#elif defined(GOKNAR_PLATFORM_UNIX)
#include <GL/gl.h>
#endif

Camera::Camera()
{
	engine->GetCameraManager()->AddCamera(this);

	Update();
}

Camera::Camera(const Vector3& position, const Vector3& gaze, const Vector3& up) : 
	Camera()
{
	position_ = position;
	forwardVector_ = gaze;
	upVector_ = up;
	leftVector_ = forwardVector_.Cross(upVector_);
	projection_ = CameraProjection::Perspective;

	Update();
}

void Camera::Init()
{
	// Set up the right vector and make forward and up vector perpenticular in case they are not
	SetLeftVector(Vector3::Cross(GetForwardVector().GetNormalized(), GetUpVector().GetNormalized()));
	SetUpVector(Vector3::Cross(GetLeftVector().GetNormalized(), GetForwardVector().GetNormalized()));
	SetForwardVector(Vector3::Cross(GetUpVector().GetNormalized(), GetLeftVector().GetNormalized()));

	if (std::abs(Vector3::Dot(forwardVector_, upVector_)) <= EPSILON)
	{
		leftVector_ = forwardVector_.Cross(upVector_);
		upVector_ = leftVector_.Cross(forwardVector_);
	}

	Update();
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
	forwardVector_ = forwardVector_.RotateVector(upVector_ * value);
	leftVector_ = forwardVector_.Cross(upVector_);

	LookAt();
}

void Camera::Pitch(float value)
{
	forwardVector_ = forwardVector_.RotateVector(leftVector_ * value);
	upVector_ = leftVector_.Cross(forwardVector_);

	LookAt();
}

void Camera::Roll(float value)
{
	leftVector_ = leftVector_.RotateVector(forwardVector_ * value);
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

void Camera::SetProjectionMatrix()
{
	float l = nearPlane_.x;
	float r = nearPlane_.y;
	float b = nearPlane_.z;
	float t = nearPlane_.w;

	// Set the projection matrix as it is orthographic
	projectionMatrix_ = Matrix( 2.f / (r - l), 0.f, 0.f, -(r + l) / (r - l),
								0.f, 2.f / (t - b), 0.f, -(t + b) / (t - b),
								0.f, 0.f, -2.f / (farDistance_ - nearDistance_), -(farDistance_ + nearDistance_) / (farDistance_ - nearDistance_),
								0.f, 0.f, 0.f, 1.f);

	if (projection_ == CameraProjection::Perspective)
	{
		// Orthographic to perspective conversion matrix
		Matrix o2p(nearDistance_, 0, 0, 0,
			0, nearDistance_, 0, 0,
			0, 0, farDistance_ + nearDistance_, farDistance_ * nearDistance_,
			0, 0, -1, 0);

		projectionMatrix_ = projectionMatrix_ * o2p;
	}
}

void Camera::LookAt()
{
	Vector3 lookAtPos = position_ + forwardVector_ * nearDistance_;

	GoknarMath::LookAt(viewingMatrix_, 
				 position_,
				 lookAtPos,
				 upVector_);
}