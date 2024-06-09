#include "pch.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Managers/WindowManager.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

#ifdef GOKNAR_PLATFORM_WINDOWS
#include <GL/GLU.h>
#elif defined(GOKNAR_PLATFORM_UNIX)
#include <GL/gl.h>
#endif

Camera::Camera()
{
	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	SetImageWidth(windowSize.x);
	SetImageHeight(windowSize.y);
	
	Update();

	engine->GetCameraManager()->AddCamera(this);
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

void Camera::Destroy()
{
	engine->GetCameraManager()->DestroyCamera(this);
}

Camera::~Camera()
{
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
	UpdateProjectionMatrix();
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

void Camera::UpdateProjectionMatrix()
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
		Matrix o2p( nearDistance_, 0.f, 0.f, 0.f,
					0.f, nearDistance_, 0.f, 0.f,
					0.f, 0.f, farDistance_ + nearDistance_, farDistance_ * nearDistance_,
					0.f, 0.f, -1.f, 0.f);

		projectionMatrix_ = projectionMatrix_ * o2p;
	}

	UpdateViewProjectionMatrix();
}

void Camera::UpdateViewProjectionMatrix()
{
	viewProjectionMatrix_ = projectionMatrix_ * viewMatrix_;
}

void Camera::LookAt()
{
	Vector3 lookAtPos = position_ + forwardVector_ * GoknarMath::Max(nearDistance_, 0.001f);

	GoknarMath::LookAt(	viewMatrix_, 
						position_,
						lookAtPos,
						upVector_);

	UpdateViewProjectionMatrix();
}

Vector2i Camera::GetScreenPositionOfWorldPosition(const Vector3& worldPosition)
{
	Vector4 screenPosition = viewProjectionMatrix_ * Vector4{ worldPosition, 1.f };
	screenPosition /= screenPosition.w;
	screenPosition = screenPosition * 0.5f + Vector4{ 0.5f, 0.5f, 0.5f, 0.f };

	return Vector2i{ (int)(screenPosition.x * imageWidth_), (int)(screenPosition.y * imageHeight_) };
}

Vector3 Camera::GetWorldDirectionAtPixel(const Vector2i& pixelCoordinate)
{
	Vector3 m = position_ + (forwardVector_ * nearDistance_);
	Vector3 q = m + leftVector_ * nearPlane_.x + upVector_ * nearPlane_.w;

	float sv = (nearPlane_.w - nearPlane_.z) * (pixelCoordinate.y + 0.5f) / imageHeight_;
	float su = (nearPlane_.y - nearPlane_.x) * (pixelCoordinate.x + 0.5f) / imageWidth_;
	Vector3 s = q + (leftVector_ * su) - (upVector_ * sv);
	Vector3 direction = s - position_;

	return direction;
}