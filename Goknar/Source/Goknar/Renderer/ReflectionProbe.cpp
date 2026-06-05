#include "pch.h"

#include "ReflectionProbe.h"

#include "Camera.h"
#include "Engine.h"
#include "Goknar/Application.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/CubemapRenderTarget.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"

#include <cmath>

ReflectionProbe::ReflectionProbe()
{
	renderTarget_ = new CubemapRenderTarget();
	engine->GetApplication()->GetMainScene()->AddReflectionProbe(this);
}

ReflectionProbe::~ReflectionProbe()
{
	engine->GetApplication()->GetMainScene()->RemoveReflectionProbe(this);
	delete renderTarget_;
}

void ReflectionProbe::PreInit()
{
	renderTarget_->Init();

	Camera* captureCamera = renderTarget_->GetCamera();
	captureCamera->SetProjection(CameraProjection::Perspective);
	captureCamera->SetCameraType(CameraType::RenderTarget);
	captureCamera->SetNearDistance(nearDistance_);
	captureCamera->SetFarDistance(captureDistance_);
	captureCamera->SetFOV(90.f);
	captureCamera->SetPosition(position_);

	UpdateViewProjectionMatrices();
	isInitialized_ = true;
}

void ReflectionProbe::Init()
{
}

void ReflectionProbe::PostInit()
{
}

void ReflectionProbe::Capture()
{
	if (!isActive_ || !isInitialized_ || !renderTarget_ || !renderTarget_->GetFrameBuffer())
	{
		return;
	}

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* previouslyActiveCamera = cameraManager->GetActiveCamera();

	cameraManager->SetActiveCamera(renderTarget_->GetCamera());

	Renderer* renderer = engine->GetRenderer();
	renderer->BeginReflectionProbeCapture(this);

	renderTarget_->GetFrameBuffer()->Bind();
	renderer->Render(RenderPassType::CubemapCapture);
	renderTarget_->GetFrameBuffer()->Unbind();

	renderTarget_->GenerateMipmaps();

	renderer->EndReflectionProbeCapture();
	cameraManager->SetActiveCamera(previouslyActiveCamera);

	needsCapture_ = false;
}

void ReflectionProbe::SetRenderPassShaderUniforms(const Shader* shader) const
{
	shader->SetMatrixArray(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::VIEW_MATRICES_ARRAY, viewProjectionMatrices_.data(), 6);
	shader->SetBool(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::HAS_REFLECTION_PROBE, false);
	shader->SetInt(SHADER_VARIABLE_NAMES::REFLECTION_PROBE::CUBEMAP, 0);
}

void ReflectionProbe::SetPosition(const Vector3& position)
{
	position_ = position;

	if (renderTarget_ && renderTarget_->GetCamera())
	{
		renderTarget_->GetCamera()->SetPosition(position_);
	}

	UpdateViewProjectionMatrices();
	needsCapture_ = true;
}

void ReflectionProbe::SetSize(const Vector3& size)
{
	const Vector3 sanitizedSize = Vector3::Max(size, Vector3(SMALLER_EPSILON));
	if (size_ == sanitizedSize)
	{
		return;
	}

	size_ = sanitizedSize;
	needsCapture_ = true;
}

bool ReflectionProbe::ContainsWorldPosition(const Vector3& worldPosition) const
{
	const Vector3 halfSize = size_ * 0.5f;
	const Vector3 minBounds = position_ - halfSize;
	const Vector3 maxBounds = position_ + halfSize;

	return minBounds <= worldPosition && worldPosition <= maxBounds;
}

void ReflectionProbe::SetNearDistance(float nearDistance)
{
	nearDistance_ = SanitizeNearDistance(nearDistance);
	captureDistance_ = SanitizeCaptureDistance(captureDistance_, nearDistance_);

	if (renderTarget_ && renderTarget_->GetCamera())
	{
		renderTarget_->GetCamera()->SetNearDistance(nearDistance_);
		renderTarget_->GetCamera()->SetFarDistance(captureDistance_);
	}

	UpdateViewProjectionMatrices();
	needsCapture_ = true;
}

void ReflectionProbe::SetCaptureDistance(float captureDistance)
{
	captureDistance_ = SanitizeCaptureDistance(captureDistance, nearDistance_);

	if (renderTarget_ && renderTarget_->GetCamera())
	{
		renderTarget_->GetCamera()->SetFarDistance(captureDistance_);
	}

	UpdateViewProjectionMatrices();
	needsCapture_ = true;
}

Texture* ReflectionProbe::GetCubemapTexture() const
{
	return renderTarget_ ? renderTarget_->GetTexture() : nullptr;
}

void ReflectionProbe::UpdateViewProjectionMatrices()
{
	if (!renderTarget_ || !renderTarget_->GetCamera())
	{
		return;
	}

	Camera* captureCamera = renderTarget_->GetCamera();
	captureCamera->SetPosition(position_);

	captureCamera->SetVectors(
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, -1.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewProjectionMatrices_[0] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ -1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, 1.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewProjectionMatrices_[1] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ 0.f, 1.f, 0.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, 1.f },
		true);
	viewProjectionMatrices_[2] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ 0.f, -1.f, 0.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, 0.f, -1.f },
		true);
	viewProjectionMatrices_[3] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ 0.f, 0.f, 1.f },
		Vector3{ 1.f, 0.f, 0.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewProjectionMatrices_[4] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ 0.f, 0.f, -1.f },
		Vector3{ -1.f, 0.f, 0.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
	viewProjectionMatrices_[5] = captureCamera->GetViewProjectionMatrix();

	captureCamera->SetVectors(
		Vector3{ 0.f, 0.f, -1.f },
		Vector3{ -1.f, 0.f, 0.f },
		Vector3{ 0.f, -1.f, 0.f },
		true);
}

float ReflectionProbe::SanitizeNearDistance(float nearDistance)
{
	return std::isfinite(nearDistance) ? GoknarMath::Max(nearDistance, SMALLER_EPSILON) : 0.1f;
}

float ReflectionProbe::SanitizeCaptureDistance(float captureDistance, float nearDistance)
{
	const float minimumCaptureDistance = SanitizeNearDistance(nearDistance) + SMALLER_EPSILON;
	return std::isfinite(captureDistance) ? GoknarMath::Max(captureDistance, minimumCaptureDistance) : 1000.f;
}