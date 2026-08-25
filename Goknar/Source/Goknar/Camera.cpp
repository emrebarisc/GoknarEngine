#include "pch.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Managers/WindowManager.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"
#include "Geometry/Box.h"

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

	if (0 < imageWidth_ && 0 < imageHeight_)
	{
		const float jitterOffsetX = temporalJitter_.x * ((r - l) / static_cast<float>(imageWidth_));
		const float jitterOffsetY = temporalJitter_.y * ((t - b) / static_cast<float>(imageHeight_));

		l += jitterOffsetX;
		r += jitterOffsetX;
		b += jitterOffsetY;
		t += jitterOffsetY;
	}

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

float Camera::GetAABBFrameCoverage(const Box& aabb) const
{
	if (imageWidth_ <= 0 || imageHeight_ <= 0)
	{
		return 0.f;
	}

	const Vector3& min = aabb.GetMin();
	const Vector3& max = aabb.GetMax();

	if (min.x <= position_.x && position_.x <= max.x &&
		min.y <= position_.y && position_.y <= max.y &&
		min.z <= position_.z && position_.z <= max.z)
	{
		return 1.f;
	}

	const Vector4 corners[8] =
	{
		viewProjectionMatrix_ * Vector4(min.x, min.y, min.z, 1.f),
		viewProjectionMatrix_ * Vector4(max.x, min.y, min.z, 1.f),
		viewProjectionMatrix_ * Vector4(min.x, max.y, min.z, 1.f),
		viewProjectionMatrix_ * Vector4(max.x, max.y, min.z, 1.f),
		viewProjectionMatrix_ * Vector4(min.x, min.y, max.z, 1.f),
		viewProjectionMatrix_ * Vector4(max.x, min.y, max.z, 1.f),
		viewProjectionMatrix_ * Vector4(min.x, max.y, max.z, 1.f),
		viewProjectionMatrix_ * Vector4(max.x, max.y, max.z, 1.f)
	};

	float minX = MAX_FLOAT;
	float maxX = MIN_FLOAT;
	float minY = MAX_FLOAT;
	float maxY = MIN_FLOAT;
	bool hasProjectedPoint = false;

	auto includeProjectedPoint = [&](const Vector4& clipPosition)
	{
		if (clipPosition.w <= SMALLER_EPSILON)
		{
			return;
		}

		const float inverseW = 1.f / clipPosition.w;
		const float x = clipPosition.x * inverseW;
		const float y = clipPosition.y * inverseW;

		minX = GoknarMath::Min(minX, x);
		maxX = GoknarMath::Max(maxX, x);
		minY = GoknarMath::Min(minY, y);
		maxY = GoknarMath::Max(maxY, y);
		hasProjectedPoint = true;
	};

	for (int i = 0; i < 8; ++i)
	{
		includeProjectedPoint(corners[i]);
	}

	for (int i = 0; i < 8; ++i)
	{
		for (int axis = 0; axis < 3; ++axis)
		{
			const int axisBit = 1 << axis;
			if ((i & axisBit) != 0)
			{
				continue;
			}

			const Vector4& start = corners[i];
			const Vector4& end = corners[i | axisBit];
			if ((start.w <= SMALLER_EPSILON) == (end.w <= SMALLER_EPSILON))
			{
				continue;
			}

			const float t = (SMALLER_EPSILON - start.w) / (end.w - start.w);
			includeProjectedPoint(start + (end - start) * t);
		}
	}

	if (!hasProjectedPoint ||
		maxX <= -1.f || 1.f <= minX ||
		maxY <= -1.f || 1.f <= minY)
	{
		return 0.f;
	}

	minX = GoknarMath::Clamp(minX, -1.f, 1.f);
	maxX = GoknarMath::Clamp(maxX, -1.f, 1.f);
	minY = GoknarMath::Clamp(minY, -1.f, 1.f);
	maxY = GoknarMath::Clamp(maxY, -1.f, 1.f);

	const float widthCoverage = (maxX - minX) * 0.5f;
	const float heightCoverage = (maxY - minY) * 0.5f;

	return GoknarMath::Clamp(widthCoverage * heightCoverage, 0.f, 1.f);
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
