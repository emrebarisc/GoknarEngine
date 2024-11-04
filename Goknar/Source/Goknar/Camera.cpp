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
	//// Near plane: -1, 0, 0, d
	//frustumPlanes_[0] = Vector4(
	//	viewProjectionMatrix_.m[3] + viewProjectionMatrix_.m[0],
	//	viewProjectionMatrix_.m[7] + viewProjectionMatrix_.m[4],
	//	viewProjectionMatrix_.m[11] + viewProjectionMatrix_.m[8],
	//	viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[12]); // Near plane (negative x)

	//// Far plane: 1, 0, 0, d
	//frustumPlanes_[1] = Vector4(
	//	viewProjectionMatrix_.m[3] - viewProjectionMatrix_.m[0],
	//	viewProjectionMatrix_.m[7] - viewProjectionMatrix_.m[4],
	//	viewProjectionMatrix_.m[11] - viewProjectionMatrix_.m[8],
	//	viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[12]); // Far plane (positive x)

	//// Left plane: 0, 1, 0, d
	//frustumPlanes_[2] = Vector4(
	//	viewProjectionMatrix_.m[3],
	//	viewProjectionMatrix_.m[7] + viewProjectionMatrix_.m[1],
	//	viewProjectionMatrix_.m[11] + viewProjectionMatrix_.m[9],
	//	viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[13]); // Left plane (positive y)

	//// Right plane: 0, -1, 0, d
	//frustumPlanes_[3] = Vector4(
	//	viewProjectionMatrix_.m[3],
	//	viewProjectionMatrix_.m[7] - viewProjectionMatrix_.m[1],
	//	viewProjectionMatrix_.m[11] - viewProjectionMatrix_.m[9],
	//	viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[13]); // Right plane (negative y)

	//// Bottom plane: 0, 0, 1, d
	//frustumPlanes_[4] = Vector4(
	//	viewProjectionMatrix_.m[3],
	//	viewProjectionMatrix_.m[7],
	//	viewProjectionMatrix_.m[11] + viewProjectionMatrix_.m[2],
	//	viewProjectionMatrix_.m[15] + viewProjectionMatrix_.m[14]); // Bottom plane (positive z)

	//// Top plane: 0, 0, -1, d
	//frustumPlanes_[5] = Vector4(
	//	viewProjectionMatrix_.m[3],
	//	viewProjectionMatrix_.m[7],
	//	viewProjectionMatrix_.m[11] - viewProjectionMatrix_.m[2],
	//	viewProjectionMatrix_.m[15] - viewProjectionMatrix_.m[14]); // Top plane (negative z)

	//for (int frustumPlaneIndex = 0; frustumPlaneIndex < 6; ++frustumPlaneIndex)
	//{
	//	Vector4& frustumPlane = frustumPlanes_[frustumPlaneIndex];
	//	float length = GoknarMath::Sqrt(frustumPlane.x * frustumPlane.x + frustumPlane.y * frustumPlane.y + frustumPlane.z * frustumPlane.z);
	//	frustumPlane.x /= length;
	//	frustumPlane.y /= length;
	//	frustumPlane.z /= length;
	//	frustumPlane.w /= length;
	//}
}

bool Camera::IsAABBVisible(const Box& aabb, const Matrix& worldTransformationMatrix) const
{
	//Vector3 localMin(aabb.GetMinX(), aabb.GetMinY(), aabb.GetMinZ());
	//Vector3 localMax(aabb.GetMaxX(), aabb.GetMaxY(), aabb.GetMaxZ());

	//Vector3 worldMin = Vector4(localMin, 1.f) * worldTransformationMatrix;
	//Vector3 worldMax = Vector4(localMax, 1.f) * worldTransformationMatrix;

	//Vector3 localCorners[8] =
	//{
	//	Vector3(localMin.x, localMin.y, localMin.z),
	//	Vector3(localMin.x, localMin.y, localMax.z),
	//	Vector3(localMin.x, localMax.y, localMin.z),
	//	Vector3(localMin.x, localMax.y, localMax.z),
	//	Vector3(localMax.x, localMin.y, localMin.z),
	//	Vector3(localMax.x, localMin.y, localMax.z),
	//	Vector3(localMax.x, localMax.y, localMin.z),
	//	Vector3(localMax.x, localMax.y, localMax.z)
	//};

	//for (int i = 1; i < 8; ++i)
	//{
	//	Vector3 transformedCorner = Vector4(localCorners[i], 0.f) * worldTransformationMatrix;
	//	worldMin = Vector3::Min(worldMin, transformedCorner);
	//	worldMax = Vector3::Max(worldMax, transformedCorner);
	//}

	//Vector3 worldCorners[8] =
	//{
	//	Vector3(worldMin.x, worldMin.y, worldMin.z),
	//	Vector3(worldMin.x, worldMin.y, worldMax.z),
	//	Vector3(worldMin.x, worldMax.y, worldMin.z),
	//	Vector3(worldMin.x, worldMax.y, worldMax.z),
	//	Vector3(worldMax.x, worldMin.y, worldMin.z),
	//	Vector3(worldMax.x, worldMin.y, worldMax.z),
	//	Vector3(worldMax.x, worldMax.y, worldMin.z),
	//	Vector3(worldMax.x, worldMax.y, worldMax.z)
	//};

	//for (int frustumPlaneIndex = 0; frustumPlaneIndex < 6; ++frustumPlaneIndex)
	//{
	//	const Vector4& plane = frustumPlanes_[frustumPlaneIndex];
	//	bool allNegative = true;
	//	for (const Vector3& corner : worldCorners)
	//	{
	//		float distance = plane.x * corner.x + plane.y * corner.y + plane.z * corner.z + plane.w;
	//		if (distance > 0)
	//		{
	//			allNegative = false;
	//			break;
	//		}
	//	}

	//	if (allNegative)
	//	{
	//		return false;
	//	}
	//}
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