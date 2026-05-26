#include "pch.h"

#include "DirectionalLight.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderTypes.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Lights/LightManager/LightManager.h"

DirectionalLight::DirectionalLight() : Light()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseDirectionalLightGUID();
	name_ = std::string(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT);

	engine->GetApplication()->GetMainScene()->AddDirectionalLight(this);
}

DirectionalLight::~DirectionalLight()
{
	engine->GetApplication()->GetMainScene()->RemoveDirectionalLight(this);
}

void DirectionalLight::PreInit()
{
	if (isShadowEnabled_)
	{
		shadowMapFrameBuffer_ = new FrameBuffer();

		shadowMapTexture_ = new Texture();
		shadowMapTexture_->SetName(SHADER_VARIABLE_NAMES::SHADOW::SHADOW_MAP_PREFIX + name_);
		shadowMapTexture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_2D);
		shadowMapTexture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_2D);
		shadowMapTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
		shadowMapTexture_->SetTextureFormat(TextureFormat::DEPTH);
		shadowMapTexture_->SetTextureMinFilter(TextureMinFilter::LINEAR);
		shadowMapTexture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
		shadowMapTexture_->SetTextureCompareMode(TextureCompareMode::COMPARE_REF_TO_TEXTURE);
		shadowMapTexture_->SetTextureCompareFunc(TextureCompareFunc::LEQUAL);

		Vector3 lightUpVector = Vector3::Cross(Vector3::Cross(direction_, direction_.RotateVectorAroundAxis(direction_.GetOrthonormalBasis(), 1.5708f)).GetNormalized(), direction_).GetNormalized();
		shadowMapRenderCamera_ = new Camera(Vector3::ZeroVector, direction_, lightUpVector);
		shadowMapRenderCamera_->SetProjection(CameraProjection::Orthographic);
		shadowMapRenderCamera_->SetCameraType(CameraType::Shadow);
		shadowMapRenderCamera_->SetImageWidth(shadowWidth_);
		shadowMapRenderCamera_->SetImageHeight(shadowHeight_);
		shadowMapRenderCamera_->SetNearPlane(Vector4{ -20.f, 20.f, -20.f, 20.f });
		shadowMapRenderCamera_->SetNearDistance(-50.f);
		shadowMapRenderCamera_->SetFarDistance(50.f);

		shadowBiasMatrix_ =
			Matrix
		{
			0.5f, 0.f, 0.f, 0.5f,
			0.f, 0.5f, 0.f, 0.5f,
			0.f, 0.f, 0.5f, 0.5f - shadowBiasValue_,
			0.f, 0.f, 0.f, 1.f
		};
		UpdateBiasedShadowMatrix();
	}

	Light::PreInit();
}

void DirectionalLight::Init()
{
	Light::Init();
}

void DirectionalLight::PostInit()
{
	Light::PostInit();
}

void DirectionalLight::SetShaderUniforms(const Shader* shader)
{
	Light::SetShaderUniforms(shader);
}

void DirectionalLight::SetShadowRenderPassShaderUniforms(const Shader* shader)
{
}

void DirectionalLight::SetDirection(const Vector3& direction)
{
	direction_ = direction.GetNormalized();

	if (isShadowEnabled_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction);
	}
}

void DirectionalLight::SetIsShadowEnabled(bool isShadowEnabled)
{
	Light::SetIsShadowEnabled(isShadowEnabled);
	if (isShadowEnabled && shadowMapRenderCamera_)
	{
		shadowMapRenderCamera_->SetForwardVector(direction_);
	}
}


void DirectionalLight::RenderShadowMap()
{
	if (!engine || !shadowMapRenderCamera_ || !shadowMapFrameBuffer_)
	{
		return;
	}

	CameraManager* cameraManager = engine->GetCameraManager();
	if (!cameraManager)
	{
		return;
	}

	Camera* mainCamera = cameraManager->GetActiveCamera();
	if (!mainCamera)
	{
		return;
	}

	const auto isValidVector = [](const Vector3& vector) -> bool
	{
		return !GoknarMath::IsNanOrInf(vector.x) && !GoknarMath::IsNanOrInf(vector.y) && !GoknarMath::IsNanOrInf(vector.z);
	};

	Vector3 lightForwardVector = shadowMapRenderCamera_->GetForwardVector().GetNormalized();
	if (!isValidVector(lightForwardVector) || lightForwardVector.SquareLength() <= SMALLER_EPSILON)
	{
		lightForwardVector = direction_.GetNormalized();
		if (!isValidVector(lightForwardVector) || lightForwardVector.SquareLength() <= SMALLER_EPSILON)
		{
			return;
		}
	}

	Vector3 lightUpVector = shadowMapRenderCamera_->GetUpVector().GetNormalized();
	if (!isValidVector(lightUpVector) ||
		lightUpVector.SquareLength() <= SMALLER_EPSILON ||
		GoknarMath::Abs(Vector3::Dot(lightForwardVector, lightUpVector)) > 1.f - SMALLER_EPSILON)
	{
		lightUpVector = lightForwardVector.GetOrthonormalBasis();
	}

	Vector3 lightLeftVector = Vector3::Cross(lightForwardVector, lightUpVector).GetNormalized();
	if (!isValidVector(lightLeftVector) || lightLeftVector.SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	lightUpVector = Vector3::Cross(lightLeftVector, lightForwardVector).GetNormalized();
	if (!isValidVector(lightUpVector) || lightUpVector.SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	Vector3 mainCameraPosition = mainCamera->GetPosition();
	Vector3 mainCameraForwardVector = mainCamera->GetForwardVector().GetNormalized();
	Vector3 mainCameraLeftVector = mainCamera->GetLeftVector().GetNormalized();
	Vector3 mainCameraUpVector = mainCamera->GetUpVector().GetNormalized();
	if (!isValidVector(mainCameraPosition) ||
		!isValidVector(mainCameraForwardVector) ||
		!isValidVector(mainCameraLeftVector) ||
		!isValidVector(mainCameraUpVector) ||
		mainCameraForwardVector.SquareLength() <= SMALLER_EPSILON ||
		mainCameraLeftVector.SquareLength() <= SMALLER_EPSILON ||
		mainCameraUpVector.SquareLength() <= SMALLER_EPSILON)
	{
		return;
	}

	Vector4 mainCameraNearPlane = mainCamera->GetNearPlane();
	if (GoknarMath::IsNanOrInf(mainCameraNearPlane.x) ||
		GoknarMath::IsNanOrInf(mainCameraNearPlane.y) ||
		GoknarMath::IsNanOrInf(mainCameraNearPlane.z) ||
		GoknarMath::IsNanOrInf(mainCameraNearPlane.w) ||
		mainCameraNearPlane.y <= mainCameraNearPlane.x ||
		mainCameraNearPlane.w <= mainCameraNearPlane.z)
	{
		return;
	}

	float mainCameraNearDistance = mainCamera->GetNearDistance();
	float mainCameraFarDistance = mainCamera->GetFarDistance();
	if (GoknarMath::IsNanOrInf(mainCameraNearDistance) || mainCameraNearDistance < 0.01f)
	{
		mainCameraNearDistance = 0.01f;
	}
	if (GoknarMath::IsNanOrInf(mainCameraFarDistance) || mainCameraFarDistance <= mainCameraNearDistance)
	{
		mainCameraFarDistance = mainCameraNearDistance + 1.f;
	}

	const float maximumShadowDistance = GoknarMath::Max(100.f, mainCameraNearDistance + 1.f);
	const float shadowDistance = GoknarMath::Clamp(mainCameraFarDistance, mainCameraNearDistance + 1.f, maximumShadowDistance);
	const float farPlaneScale = mainCamera->GetProjection() == CameraProjection::Perspective ? shadowDistance / mainCameraNearDistance : 1.f;
	const Vector4 shadowFarPlane
	{
		mainCameraNearPlane.x * farPlaneScale,
		mainCameraNearPlane.y * farPlaneScale,
		mainCameraNearPlane.z * farPlaneScale,
		mainCameraNearPlane.w * farPlaneScale
	};

	const Vector3 nearPlaneCenter = mainCameraPosition + mainCameraForwardVector * mainCameraNearDistance;
	const Vector3 farPlaneCenter = mainCameraPosition + mainCameraForwardVector * shadowDistance;
	const Vector3 frustumCorners[8]
	{
		nearPlaneCenter - mainCameraLeftVector * mainCameraNearPlane.x + mainCameraUpVector * mainCameraNearPlane.z,
		nearPlaneCenter - mainCameraLeftVector * mainCameraNearPlane.y + mainCameraUpVector * mainCameraNearPlane.z,
		nearPlaneCenter - mainCameraLeftVector * mainCameraNearPlane.x + mainCameraUpVector * mainCameraNearPlane.w,
		nearPlaneCenter - mainCameraLeftVector * mainCameraNearPlane.y + mainCameraUpVector * mainCameraNearPlane.w,
		farPlaneCenter - mainCameraLeftVector * shadowFarPlane.x + mainCameraUpVector * shadowFarPlane.z,
		farPlaneCenter - mainCameraLeftVector * shadowFarPlane.y + mainCameraUpVector * shadowFarPlane.z,
		farPlaneCenter - mainCameraLeftVector * shadowFarPlane.x + mainCameraUpVector * shadowFarPlane.w,
		farPlaneCenter - mainCameraLeftVector * shadowFarPlane.y + mainCameraUpVector * shadowFarPlane.w
	};

	float minX = MAX_FLOAT;
	float maxX = MIN_FLOAT;
	float minY = MAX_FLOAT;
	float maxY = MIN_FLOAT;
	float minZ = MAX_FLOAT;
	float maxZ = MIN_FLOAT;

	for (const Vector3& frustumCorner : frustumCorners)
	{
		if (!isValidVector(frustumCorner))
		{
			return;
		}

		const float x = Vector3::Dot(lightLeftVector, frustumCorner);
		const float y = Vector3::Dot(lightUpVector, frustumCorner);
		const float z = Vector3::Dot(lightForwardVector, frustumCorner);
		if (GoknarMath::IsNanOrInf(x) || GoknarMath::IsNanOrInf(y) || GoknarMath::IsNanOrInf(z))
		{
			return;
		}

		minX = GoknarMath::Min(minX, x);
		maxX = GoknarMath::Max(maxX, x);
		minY = GoknarMath::Min(minY, y);
		maxY = GoknarMath::Max(maxY, y);
		minZ = GoknarMath::Min(minZ, z);
		maxZ = GoknarMath::Max(maxZ, z);
	}

	const float shadowMapWidth = static_cast<float>(shadowMapRenderCamera_->GetImageWidth());
	const float shadowMapHeight = static_cast<float>(shadowMapRenderCamera_->GetImageHeight());
	const float minShadowExtent = 1.f;
	float shadowExtentX = GoknarMath::Max(maxX - minX, minShadowExtent);
	float shadowExtentY = GoknarMath::Max(maxY - minY, minShadowExtent);
	const float shadowDepth = GoknarMath::Max(maxZ - minZ, minShadowExtent);
	const float shadowPadding = GoknarMath::Max(shadowExtentX, shadowExtentY) * 0.05f;

	shadowExtentX += shadowPadding * 2.f;
	shadowExtentY += shadowPadding * 2.f;

	float centerX = (minX + maxX) * 0.5f;
	float centerY = (minY + maxY) * 0.5f;
	if (0.f < shadowMapWidth)
	{
		const float texelSizeX = shadowExtentX / shadowMapWidth;
		if (SMALLER_EPSILON < texelSizeX && !GoknarMath::IsNanOrInf(texelSizeX))
		{
			centerX = GoknarMath::Floor(centerX / texelSizeX) * texelSizeX;
		}
	}
	if (0.f < shadowMapHeight)
	{
		const float texelSizeY = shadowExtentY / shadowMapHeight;
		if (SMALLER_EPSILON < texelSizeY && !GoknarMath::IsNanOrInf(texelSizeY))
		{
			centerY = GoknarMath::Floor(centerY / texelSizeY) * texelSizeY;
		}
	}

	const float cameraDepthPadding = GoknarMath::Max(shadowDepth * 0.1f, 1.f);
	const Vector3 shadowCameraPosition =
		lightLeftVector * centerX +
		lightUpVector * centerY +
		lightForwardVector * (minZ - cameraDepthPadding);

	shadowMapRenderCamera_->SetVectors(lightForwardVector, lightLeftVector, lightUpVector, false);
	shadowMapRenderCamera_->SetNearPlane(Vector4(
		minX - centerX - shadowPadding,
		maxX - centerX + shadowPadding,
		minY - centerY - shadowPadding,
		maxY - centerY + shadowPadding));
	shadowMapRenderCamera_->SetNearDistance(0.f);
	shadowMapRenderCamera_->SetFarDistance(shadowDepth + cameraDepthPadding * 2.f);
	shadowMapRenderCamera_->SetPosition(shadowCameraPosition);
	UpdateBiasedShadowMatrix();

	cameraManager->SetActiveCamera(shadowMapRenderCamera_);
	shadowMapFrameBuffer_->Bind();

	Renderer* renderer = engine->GetRenderer();
	if (!renderer)
	{
		shadowMapFrameBuffer_->Unbind();
		cameraManager->SetActiveCamera(mainCamera);
		return;
	}

	renderer->Render(RenderPassType::Shadow);

	// For outputing only!
	//shadowMapTexture_->ReadFromFrameBuffer(directionalLight->GetShadowMapFBO());
	//shadowMapTexture_->Save(CONTENT_DIR + directionalLight->GetName() + "FrameBufferTexture.png");

	shadowMapFrameBuffer_->Unbind();
	cameraManager->SetActiveCamera(mainCamera);

	EXIT_ON_GRAPHICS_API_ERROR("DirectionalLight::RenderShadowMap");
}
