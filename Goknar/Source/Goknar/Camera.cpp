#include "pch.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Managers/WindowManager.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"
#include "Geometry/Box.h"

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

Camera::Camera(const Vector3& position, const Vector3& forward, const Vector3& up) :
	Camera()
{
	position_ = position;
	forwardVector_ = forward;
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
	SetPosition(position_ + forwardVector_ * value);
}

void Camera::MoveUpward(float value)
{
	SetPosition(position_ + upVector_ * value);
}

void Camera::MoveRight(float value)
{
	SetPosition(position_ + leftVector_ * value);
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
	projectionMatrix_ = Matrix(2.f / (r - l), 0.f, 0.f, -(r + l) / (r - l),
		0.f, 2.f / (t - b), 0.f, -(t + b) / (t - b),
		0.f, 0.f, -2.f / (farDistance_ - nearDistance_), -(farDistance_ + nearDistance_) / (farDistance_ - nearDistance_),
		0.f, 0.f, 0.f, 1.f);

	if (projection_ == CameraProjection::Perspective)
	{
		// Orthographic to perspective conversion matrix
		Matrix o2p(nearDistance_, 0.f, 0.f, 0.f,
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

	UpdateFrustumPlanes();
}

void Camera::LookAt()
{
	Vector3 lookAtPos = position_ + forwardVector_ * GoknarMath::Max(nearDistance_, 0.001f);

	GoknarMath::LookAt(viewMatrix_,
		position_,
		lookAtPos,
		upVector_);

	UpdateViewProjectionMatrix();
}

void Camera::UpdateFrustumPlanes()
{
	// Left plane (w + x)
	frustumPlanes_[0] = Vector4(
		viewProjectionMatrix_.m[12] + viewProjectionMatrix_.m[0],
		viewProjectionMatrix_.m[13] + viewProjectionMatrix_.m[1],
		viewProjectionMatrix_.m[14] + viewProjectionMatrix_.m[2],
		viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[3]);

	// Right plane (w - x)
	frustumPlanes_[1] = Vector4(
		viewProjectionMatrix_.m[12] - viewProjectionMatrix_.m[0],
		viewProjectionMatrix_.m[13] - viewProjectionMatrix_.m[1],
		viewProjectionMatrix_.m[14] - viewProjectionMatrix_.m[2],
		viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[3]);

	// Bottom plane (w + y)
	frustumPlanes_[2] = Vector4(
		viewProjectionMatrix_.m[12] + viewProjectionMatrix_.m[4],
		viewProjectionMatrix_.m[13] + viewProjectionMatrix_.m[5],
		viewProjectionMatrix_.m[14] + viewProjectionMatrix_.m[6],
		viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[7]);

	// Top plane (w - y)
	frustumPlanes_[3] = Vector4(
		viewProjectionMatrix_.m[12] - viewProjectionMatrix_.m[4],
		viewProjectionMatrix_.m[13] - viewProjectionMatrix_.m[5],
		viewProjectionMatrix_.m[14] - viewProjectionMatrix_.m[6],
		viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[7]);

	// Near plane (w + z)
	frustumPlanes_[4] = Vector4(
		viewProjectionMatrix_.m[12] + viewProjectionMatrix_.m[8],
		viewProjectionMatrix_.m[13] + viewProjectionMatrix_.m[9],
		viewProjectionMatrix_.m[14] + viewProjectionMatrix_.m[10],
		viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[11]);

	// Far plane (w - z)
	frustumPlanes_[5] = Vector4(
		viewProjectionMatrix_.m[12] - viewProjectionMatrix_.m[8],
		viewProjectionMatrix_.m[13] - viewProjectionMatrix_.m[9],
		viewProjectionMatrix_.m[14] - viewProjectionMatrix_.m[10],
		viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[11]);

	for (int i = 0; i < 6; ++i)
	{
		Vector4& plane = frustumPlanes_[i];
		float length = GoknarMath::Sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
		plane.x /= length;
		plane.y /= length;
		plane.z /= length;
		plane.w /= length;
	}
}

bool Camera::IsAABBVisible(const Box& aabb, const Matrix& worldTransformationMatrix) const
{
	Vector3 localCenter = (aabb.GetMax() + aabb.GetMin()) * 0.5f;
	Vector3 localExtents = (aabb.GetMax() - aabb.GetMin()) * 0.5f;

	Vector4 worldCenter4 = worldTransformationMatrix * Vector4(localCenter, 1.f);
	Vector3 worldCenter(worldCenter4);

	Vector3 axisX(worldTransformationMatrix.m[0], worldTransformationMatrix.m[4], worldTransformationMatrix.m[8]);
	axisX *= localExtents.x;

	Vector3 axisY(worldTransformationMatrix.m[1], worldTransformationMatrix.m[5], worldTransformationMatrix.m[9]);
	axisY *= localExtents.y;

	Vector3 axisZ(worldTransformationMatrix.m[2], worldTransformationMatrix.m[6], worldTransformationMatrix.m[10]);
	axisZ *= localExtents.z;

	for (int i = 0; i < 6; ++i)
	{
		const Vector4& plane = frustumPlanes_[i];

		float distanceToCenter = 
			plane.x * worldCenter.x +
			plane.y * worldCenter.y +
			plane.z * worldCenter.z +
			plane.w;

		float projectedRadius =
			GoknarMath::Abs(plane.x * axisX.x + plane.y * axisX.y + plane.z * axisX.z) +
			GoknarMath::Abs(plane.x * axisY.x + plane.y * axisY.y + plane.z * axisY.z) +
			GoknarMath::Abs(plane.x * axisZ.x + plane.y * axisZ.y + plane.z * axisZ.z);

		if (distanceToCenter < -projectedRadius)
		{
			return false;
		}
	}

	return true;
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
	Vector3 q = m - leftVector_ * nearPlane_.x + upVector_ * nearPlane_.w;

	float su, sv;
	Vector3 s, ray, d;

	sv = (nearPlane_.w - nearPlane_.z) * (pixelCoordinate.y + 0.5f) / imageHeight_;
	su = (nearPlane_.y - nearPlane_.x) * (pixelCoordinate.x + 0.5f) / imageWidth_;
	s = q - (leftVector_ * su) - (upVector_ * sv);
	d = s - position_;

	return d;
}