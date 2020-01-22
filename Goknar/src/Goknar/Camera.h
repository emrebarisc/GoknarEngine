#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Goknar/Core.h"
#include "Goknar/Matrix.h"

enum class GOKNAR_API CameraProjection : unsigned char
{
	Orthographic,
	Perspective
};

class GOKNAR_API Camera
{
public:
	Camera() : 
		viewingMatrix_(Matrix::IdentityMatrix),
		position_(Vector3::ZeroVector),
		forwardVector_(Vector3::ForwardVector),
		leftVector_(Vector3::LeftVector),
		upVector_(Vector3::UpVector),
		nearPlane_(0.1f),
		nearDistance_(0.1f),
		farDistance_(100.f),
		imageWidth_(1024),
		imageHeight_(768),
		projection_(CameraProjection::Perspective)
	{
		SetProjectionMatrix();
		LookAt();
	}

	Camera(const Vector3& position, const Vector3& forward, const Vector3& up);

	Camera(const Camera* rhs)
	{
		if (rhs != this)
		{
			position_ = rhs->position_;
			forwardVector_ = rhs->forwardVector_;
			upVector_ = rhs->upVector_;
			leftVector_ = rhs->leftVector_;
			nearPlane_ = rhs->nearPlane_;
			nearDistance_ = rhs->nearDistance_;
			farDistance_ = rhs->farDistance_;
			imageWidth_ = rhs->imageWidth_;
			imageHeight_ = rhs->imageHeight_;
			projection_ = rhs->projection_;
			projectionMatrix_ = rhs->projectionMatrix_;
			viewingMatrix_ = rhs->viewingMatrix_;
		}
		else
		{
			Camera();
		}
	}

	~Camera()
	{

	}

	void Init();
	void Update();

	void MoveForward(float value);
	void MoveUpward(float value);
	void MoveRight(float value);
	void Yaw(float value);
	void Pitch(float value);
	void Roll(float value);
	void RotateAbout(const Vector3& axis, float angle);

	const Matrix& GetViewingMatrix() const
	{
		return viewingMatrix_;
	}

	const Matrix& GetProjectionMatrix() const
	{
		return projectionMatrix_;
	}

	void SetPosition(const Vector3& position)
	{
		position_ = position;
		LookAt();
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
		leftVector_ = rightVector;
	}

	const Vector3& GetRightVector() const
	{
		return leftVector_;
	}

	void SetImageWidth(int width)
	{
		imageWidth_ = width;
	}

	int GetImageWidth() const
	{
		return imageWidth_;
	}

	void SetImageHeight(int height)
	{
		imageHeight_ = height;
	}

	int GetImageHeight() const
	{
		return imageHeight_;
	}

	void SetNearPlane(const Vector4& nearPlane)
	{
		nearPlane_ = nearPlane;
	}

	const Vector4& GetNearPlane() const
	{
		return nearPlane_;
	}

	void SetNearDistance(float nearDistance)
	{
		nearDistance_ = nearDistance;
	}

	float GetNearDistance() const
	{
		return nearDistance_;
	}

	void SetFarDistance(float farDistance)
	{
		farDistance_ = farDistance;
	}

	float GetFarDistance() const
	{
		return farDistance_;
	}

	void InitMatrices()
	{
		LookAt();
		SetProjectionMatrix();
	}

	void SetProjectionMatrix()
	{
		float l = nearPlane_.x;
		float r = nearPlane_.y;
		float b = nearPlane_.z;
		float t = nearPlane_.w;

		// Set the projection matrix as it is orthographic
		projectionMatrix_ = Matrix( 2 / (r - l), 0.f, 0.f, -(r + l) / (r - l),
									0.f, 2 / (t - b), 0.f, -(t + b) / (t - b),
									0.f, 0.f, -2 / (farDistance_ - nearDistance_), -(farDistance_ + nearDistance_) / (farDistance_ - nearDistance_),
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

	float GetViewingMatrixPointer()
	{
		return viewingMatrix_[0];
	}

	float GetProjectionMatrixPointer()
	{
		return projectionMatrix_[0];
	}

	void SetProjection(CameraProjection projection)
	{
		projection_ = projection;
	}

	CameraProjection GetProjection() const
	{
		return projection_;
	}

protected:

private:
	void LookAt();

	Matrix viewingMatrix_;
	Matrix projectionMatrix_;

	// Left Right Bottom Top
	Vector4 nearPlane_;

	Vector3 position_;
	Vector3 forwardVector_;
	Vector3 upVector_;
	Vector3 leftVector_;

	float nearDistance_;
	float farDistance_;
	int imageWidth_, imageHeight_;

	CameraProjection projection_;
};

#endif