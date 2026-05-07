#ifndef __REFLECTIONPROBE_H__
#define __REFLECTIONPROBE_H__

#include "Goknar/Core.h"
#include "Math/GoknarMath.h"
#include "Math/Matrix.h"

#include <array>

class CubemapRenderTarget;
class Shader;
class Texture;

class GOKNAR_API ReflectionProbe
{
public:
	ReflectionProbe();
	~ReflectionProbe();

	void PreInit();
	void Init();
	void PostInit();

	void Capture();
	void SetRenderPassShaderUniforms(const Shader* shader) const;

	void SetPosition(const Vector3& position);

	const Vector3& GetPosition() const
	{
		return position_;
	}

	void SetSize(const Vector3& size);

	const Vector3& GetSize() const
	{
		return size_;
	}

	bool ContainsWorldPosition(const Vector3& worldPosition) const;

	void SetNearDistance(float nearDistance);

	float GetNearDistance() const
	{
		return nearDistance_;
	}

	void SetCaptureDistance(float captureDistance);

	float GetCaptureDistance() const
	{
		return captureDistance_;
	}

	void SetCaptureEveryFrame(bool captureEveryFrame)
	{
		captureEveryFrame_ = captureEveryFrame;
	}

	bool GetCaptureEveryFrame() const
	{
		return captureEveryFrame_;
	}

	void SetNeedsCapture(bool needsCapture)
	{
		needsCapture_ = needsCapture;
	}

	bool GetNeedsCapture() const
	{
		return needsCapture_;
	}

	void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetIsActive() const
	{
		return isActive_;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	CubemapRenderTarget* GetRenderTarget() const
	{
		return renderTarget_;
	}

	Texture* GetCubemapTexture() const;

	const std::array<Matrix, 6>& GetViewProjectionMatrices() const
	{
		return viewProjectionMatrices_;
	}

private:
	void UpdateViewProjectionMatrices();

	CubemapRenderTarget* renderTarget_{ nullptr };
	std::array<Matrix, 6> viewProjectionMatrices_{};

	Vector3 position_{ Vector3::ZeroVector };
	Vector3 size_{ Vector3(10.f) };

	float nearDistance_{ 0.1f };
	float captureDistance_{ 100.f };

	bool captureEveryFrame_{ true };
	bool needsCapture_{ true };
	bool isActive_{ true };
	bool isInitialized_{ false };
};

#endif
