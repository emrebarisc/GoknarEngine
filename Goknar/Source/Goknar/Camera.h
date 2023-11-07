#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

#include "Goknar/Log.h"

enum class GOKNAR_API CameraProjection : unsigned char
{
	Orthographic,
	Perspective
};

enum class GOKNAR_API CameraType : unsigned char
{
	Scene,
	Shadow
};

class GOKNAR_API Camera
{
public:
	Camera();
	Camera(const Vector3& position, const Vector3& forward, const Vector3& up);
	Camera(const Camera* rhs) : Camera()
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
	}

	virtual ~Camera()
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

	inline const Matrix& GetViewingMatrix() const
	{
		return viewingMatrix_;
	}

	inline const Matrix& GetProjectionMatrix() const
	{
		return projectionMatrix_;
	}

	inline const Matrix& GetViewProjectionMatrix() const
	{
		return viewProjectionMatrix_;
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
		if (EPSILON < Vector3::Distance(forwardVector_, forwardVector))
		{
			forwardVector_ = forwardVector;
			leftVector_ = forwardVector_.Cross(upVector_);
			upVector_ = leftVector_.Cross(forwardVector_);

			LookAt();
		}
	}

	const Vector3& GetForwardVector() const
	{
		return forwardVector_;
	}

	void SetUpVector(const Vector3& upVector)
	{
		if (EPSILON < Vector3::Distance(upVector_, upVector))
		{
			upVector_ = upVector;
			forwardVector_ = upVector_.Cross(leftVector_);
			leftVector_ = forwardVector_.Cross(upVector_);

			LookAt();
		}
	}

	const Vector3& GetUpVector() const
	{
		return upVector_;
	}

	void SetLeftVector(const Vector3& leftVector)
	{
		if (EPSILON < Vector3::Distance(leftVector_, leftVector))
		{
			leftVector_ = leftVector;
			forwardVector_ = upVector_.Cross(leftVector_);
			upVector_ = leftVector_.Cross(forwardVector_);

			LookAt();
		}
	}

	const Vector3& GetLeftVector() const
	{
		return leftVector_;
	}

	void SetImageWidth(int width)
	{
		imageWidth_ = width;
		UpdateNearPlane();
	}

	int GetImageWidth() const
	{
		return imageWidth_;
	}

	void SetImageHeight(int height)
	{
		imageHeight_ = height;
		UpdateNearPlane();
	}

	int GetImageHeight() const
	{
		return imageHeight_;
	}

	void SetNearPlane(const Vector4& nearPlane)
	{
		nearPlane_ = nearPlane;
		UpdateProjectionMatrix();
	}

	const Vector4& GetNearPlane() const
	{
		return nearPlane_;
	}

	void SetNearDistance(float nearDistance)
	{
		nearDistance_ = nearDistance;
		UpdateProjectionMatrix();
	}

	float GetNearDistance() const
	{
		return nearDistance_;
	}

	void SetFarDistance(float farDistance)
	{
		farDistance_ = farDistance;
		UpdateProjectionMatrix();
	}

	void UpdateNearPlane()
	{
		nearPlane_ = Vector4(-0.5f, 0.5f, -(float)imageHeight_ / imageWidth_ * 0.5f, (float)imageHeight_ / imageWidth_ * 0.5f);
		UpdateProjectionMatrix();
	}

	void SetFOV(float fovInDegrees)
	{
		float resolutionProportion = (float)imageWidth_ / imageHeight_;

		float halfOfFovY = fovInDegrees * 0.5f;
		float top, bottom, left, right;
		top = nearDistance_ * tan(DEGREE_TO_RADIAN(halfOfFovY));
		bottom = -top;
		left = bottom * resolutionProportion;
		right = top * resolutionProportion;

		SetNearPlane(Vector4(left, right, bottom, top));
	}

	float GetFarDistance() const
	{
		return farDistance_;
	}

	void UpdateProjectionMatrix();
	void UpdateViewProjectionMatrix();

	float* GetViewingMatrixPointer()
	{
		return &viewingMatrix_.m[0];
	}

	float* GetProjectionMatrixPointer()
	{
		return &projectionMatrix_.m[0];
	}

	void SetProjection(CameraProjection projection)
	{
		if (projection_ != projection)
		{
			projection_ = projection;
			UpdateProjectionMatrix();
		}
	}

	CameraProjection GetProjection() const
	{
		return projection_;
	}

	void SetCameraType(CameraType cameraType)
	{
		cameraType_ = cameraType;
	}

	CameraType GetCameraType() const
	{
		return cameraType_;
	}

	void SetVectors(Vector3 forward, Vector3 left, Vector3 up)
	{
		forwardVector_ = forward;
		leftVector_ = left;
		upVector_ = up;

		LookAt();
	}

protected:

private:
	void LookAt();

	Matrix viewingMatrix_{ Matrix::IdentityMatrix };
	Matrix projectionMatrix_{ Matrix::IdentityMatrix };
	Matrix viewProjectionMatrix_{ Matrix::IdentityMatrix };

	// Left Right Bottom Top
	Vector4 nearPlane_{ Vector4(-0.5f, 0.5f, -0.28125f, 0.28125f) };

	Vector3 position_{ Vector3::ZeroVector };
	Vector3 forwardVector_{ Vector3::ForwardVector };
	Vector3 upVector_{ Vector3::UpVector };
	Vector3 leftVector_{ Vector3::LeftVector };

	float nearDistance_{ 1.f };
	float farDistance_{ 1000.f };
	int imageWidth_{ 1600 };
	int imageHeight_{ 900 };

	CameraProjection projection_{ CameraProjection::Perspective };
	CameraType cameraType_{ CameraType::Scene };
};

#endif