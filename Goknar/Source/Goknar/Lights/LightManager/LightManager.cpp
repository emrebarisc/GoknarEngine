#include "pch.h"

#include "LightManager.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

#include "Goknar/IO/IOManager.h"
#include "Goknar/Renderer/Texture.h"

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
<<<<<<< HEAD
	lightStorageBuffer_.Destroy();

	if (directionalLightViewMatrixUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(directionalLightViewMatrixUniformBufferId_);
=======
	if (directionalLightUniformBufferId_)
	{
		glDeleteBuffers(1, &directionalLightUniformBufferId_);
	}

	if (directionalLightViewMatrixUniformBufferId_)
	{
		glDeleteBuffers(1, &directionalLightViewMatrixUniformBufferId_);
	}

	if (pointLightUniformBufferId_)
	{
		glDeleteBuffers(1, &pointLightUniformBufferId_);
	}

	if (spotLightUniformBufferId_)
	{
		glDeleteBuffers(1, &spotLightUniformBufferId_);
>>>>>>> master
	}

	if (spotLightViewMatrixUniformBufferId_)
	{
<<<<<<< HEAD
		engine->GetGraphicsAPI()->DeleteBuffer(spotLightViewMatrixUniformBufferId_);
=======
		glDeleteBuffers(1, &spotLightViewMatrixUniformBufferId_);
>>>>>>> master
	}
}

void LightManager::PreInit()
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	lightStorageBuffer_.Create(sizeof(LightBufferInfo), &lightBufferInfo, GraphicsBufferUsage::DynamicDraw);
	lightStorageBuffer_.BindToBindingPoint(ShaderBindingPoints::ShaderStorage::LIGHT_DATA);

	{
		directionalLightViewMatrixUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightViewMatrixUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * MAX_DIRECTIONAL_LIGHT_COUNT, NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, directionalLightViewMatrixUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);
	}

	{
		spotLightViewMatrixUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, spotLightViewMatrixUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * MAX_SPOT_LIGHT_COUNT, NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, spotLightViewMatrixUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);
	}

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::PreInit");
}

void LightManager::Init()
{
}

void LightManager::PostInit()
{
	CollectDirectionalLightData();
	CollectPointLightData();
	CollectSpotLightData();
	UploadLightDataToGPU();

	isInitialized_ = true;
}

void LightManager::CollectDirectionalLightData()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> directionalLights = mainScene->GetDirectionalLights();

	int directionalLightIndex = 0;
	std::vector<DirectionalLight*>::const_iterator directionalLightIterator = directionalLights.cbegin();
	while (directionalLightIndex < static_cast<int>(MAX_DIRECTIONAL_LIGHT_COUNT) && directionalLightIterator != directionalLights.cend())
	{
		DirectionalLight* directionalLight = *directionalLightIterator;

		if (directionalLight && directionalLight->GetIsActive())
		{
			lightBufferInfo.directionalLightInfo[directionalLightIndex].direction = directionalLight->GetDirection();
			lightBufferInfo.directionalLightInfo[directionalLightIndex].intensity = directionalLight->GetIntensity() * directionalLight->GetColor();
			lightBufferInfo.directionalLightInfo[directionalLightIndex].isCastingShadow = directionalLight->GetIsShadowEnabled();
			lightBufferInfo.directionalLightInfo[directionalLightIndex].shadowIntensity = directionalLight->GetShadowIntensity();

			directionalLight->SetUniformBufferIndex(directionalLightIndex);
			++directionalLightIndex;
		}

		++directionalLightIterator;
	}
	directionalLightCount_ = directionalLightIndex;
	lightDataDirty_ = true;
}

void LightManager::CollectPointLightData()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();

	int pointLightIndex = 0;
	std::vector<PointLight*>::const_iterator pointLightIterator = pointLights.cbegin();
	while (pointLightIndex < static_cast<int>(MAX_POINT_LIGHT_COUNT) && pointLightIterator != pointLights.cend())
	{
		PointLight* pointLight = *pointLightIterator;

		if (pointLight && pointLight->GetIsActive())
		{
			lightBufferInfo.pointLightInfo[pointLightIndex].position = pointLight->GetPosition();
			lightBufferInfo.pointLightInfo[pointLightIndex].radius = pointLight->GetRadius();
			lightBufferInfo.pointLightInfo[pointLightIndex].intensity = pointLight->GetIntensity() * pointLight->GetColor();
			lightBufferInfo.pointLightInfo[pointLightIndex].isCastingShadow = pointLight->GetIsShadowEnabled();
			lightBufferInfo.pointLightInfo[pointLightIndex].shadowIntensity = pointLight->GetShadowIntensity();

			pointLight->SetUniformBufferIndex(pointLightIndex);
			++pointLightIndex;
		}

		++pointLightIterator;
	}
	pointLightCount_ = pointLightIndex;
	lightDataDirty_ = true;
}

void LightManager::CollectSpotLightData()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();

	int spotLightIndex = 0;
	std::vector<SpotLight*>::const_iterator spotLightIterator = spotLights.cbegin();
	while (spotLightIndex < static_cast<int>(MAX_SPOT_LIGHT_COUNT) && spotLightIterator != spotLights.cend())
	{
		SpotLight* spotLight = *spotLightIterator;

		if (spotLight && spotLight->GetIsActive())
		{

			lightBufferInfo.spotLightInfo[spotLightIndex].position = spotLight->GetPosition();
			lightBufferInfo.spotLightInfo[spotLightIndex].coverageAngle = spotLight->GetCoverageAngle();
			lightBufferInfo.spotLightInfo[spotLightIndex].direction = spotLight->GetDirection();
			lightBufferInfo.spotLightInfo[spotLightIndex].falloffAngle = spotLight->GetFalloffAngle();
			lightBufferInfo.spotLightInfo[spotLightIndex].intensity = spotLight->GetIntensity() * spotLight->GetColor();
			lightBufferInfo.spotLightInfo[spotLightIndex].isCastingShadow = spotLight->GetIsShadowEnabled();
			lightBufferInfo.spotLightInfo[spotLightIndex].shadowIntensity = spotLight->GetShadowIntensity();

			spotLight->SetUniformBufferIndex(spotLightIndex);
			++spotLightIndex;
		}

		++spotLightIterator;
	}
	spotLightCount_ = spotLightIndex;
	lightDataDirty_ = true;
}

void LightManager::UploadLightDataToGPU()
{
	lightStorageBuffer_.Upload(&lightBufferInfo, sizeof(lightBufferInfo), GraphicsBufferUsage::DynamicDraw, true);
	lightStorageBuffer_.BindToBindingPoint(ShaderBindingPoints::ShaderStorage::LIGHT_DATA);
	lightDataDirty_ = false;

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::UploadLightDataToGPU");
}

void LightManager::UploadLightDataIfDirty()
{
	if (!lightDataDirty_)
	{
		return;
	}

	CollectDirectionalLightData();
	CollectPointLightData();
	CollectSpotLightData();
	UploadLightDataToGPU();
}

void LightManager::RenderShadowMaps()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	// Only draw the depth buffer
	engine->GetGraphicsAPI()->SetColorMask(false, false, false, false);

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();
	size_t pointLightCount = pointLights.size();
	for (size_t i = 0; i < pointLightCount; i++)
	{
		PointLight* pointLight = pointLights[i];

		if (!pointLight->GetIsActive())
		{
			continue;
		}

		if (!pointLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = pointLight;
		pointLight->RenderShadowMap();
	}

	if(mainCamera)
	{
		const std::vector<DirectionalLight*> directionalLights = mainScene->GetDirectionalLights();
		size_t directionalLightCount = directionalLights.size();
		for (size_t i = 0; i < directionalLightCount; i++)
		{
			DirectionalLight* directionalLight = directionalLights[i];
			if (!directionalLight->GetIsActive())
			{
				continue;
			}

			cameraManager->SetActiveCamera(mainCamera);

			if (!directionalLight->GetIsShadowEnabled())
			{
				continue;
			}

			currentlyRenderingLight_ = directionalLight;
			directionalLight->RenderShadowMap();
		}
	}

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();
	size_t spotLightCount = spotLights.size();
	for (size_t i = 0; i < spotLightCount; i++)
	{
		SpotLight* spotLight = spotLights[i];

		if (!spotLight->GetIsActive())
		{
			continue;
		}

		if (!spotLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = spotLight;
		spotLight->RenderShadowMap();
	}

	engine->GetGraphicsAPI()->SetColorMask(true, true, true, true);
	cameraManager->SetActiveCamera(mainCamera);

	BindShadowViewProjectionMatrices();

	UpdateLights();

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::RenderShadowMaps");
}

void LightManager::SetShadowRenderPassShaderUniforms(const Shader* shader) const
{
	if (!currentlyRenderingLight_)
	{
		return;
	}

	currentlyRenderingLight_->SetShadowRenderPassShaderUniforms(shader);
}

void LightManager::BindLightUniforms(Shader* shader)
{
	if (!shader)
	{
		return;
	}

	UploadLightDataIfDirty();
	lightStorageBuffer_.BindToBindingPoint(ShaderBindingPoints::ShaderStorage::LIGHT_DATA);

	shader->Use();
	shader->SetInt(SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE, directionalLightCount_);
	shader->SetInt(SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_COUNT_IN_USE_VARIABLE, pointLightCount_);
	shader->SetInt(SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_COUNT_IN_USE_VARIABLE, spotLightCount_);
}

void LightManager::BindShadowViewProjectionMatrices()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> directionalLights = mainScene->GetDirectionalLights();
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightViewMatrixUniformBufferId_);
	int directionalLightIndex = 0;
	for (DirectionalLight* directionalLight : directionalLights)
	{
		if (!(directionalLightIndex < static_cast<int>(MAX_DIRECTIONAL_LIGHT_COUNT)))
		{
			break;
		}

		if (!directionalLight || !directionalLight->GetIsActive())
		{
			continue;
		}

		graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * directionalLightIndex, sizeof(Matrix), &directionalLight->GetBiasedShadowViewProjectionMatrix());
		++directionalLightIndex;
	}
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, spotLightViewMatrixUniformBufferId_);
	int spotLightIndex = 0;
	for (SpotLight* spotLight : spotLights)
	{
		if (!(spotLightIndex < static_cast<int>(MAX_SPOT_LIGHT_COUNT)))
		{
			break;
		}

		if (!spotLight || !spotLight->GetIsActive())
		{
			continue;
		}

		graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * spotLightIndex, sizeof(Matrix), &spotLight->GetBiasedShadowViewProjectionMatrix());
		++spotLightIndex;
	}
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::BindShadowViewProjectionMatrices");
}

void LightManager::OnDirectionalLightAdded(DirectionalLight* directionalLight)
{
	(void)directionalLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectDirectionalLightData();
	UploadLightDataToGPU();
}

void LightManager::OnDirectionalLightRemoved(DirectionalLight* directionalLight)
{
	(void)directionalLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectDirectionalLightData();
	UploadLightDataToGPU();
}

void LightManager::OnPointLightAdded(PointLight* pointLight)
{
	(void)pointLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectPointLightData();
	UploadLightDataToGPU();
}

void LightManager::OnPointLightRemoved(PointLight* pointLight)
{
	(void)pointLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectPointLightData();
	UploadLightDataToGPU();
}

void LightManager::OnSpotLightAdded(SpotLight* spotLight)
{
	(void)spotLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectSpotLightData();
	UploadLightDataToGPU();
}

void LightManager::OnSpotLightRemoved(SpotLight* spotLight)
{
	(void)spotLight;

	if (!isInitialized_)
	{
		lightDataDirty_ = true;
		return;
	}

	CollectSpotLightData();
	UploadLightDataToGPU();
}

void LightManager::UpdateLights()
{
	if (!lightStorageBuffer_.IsCreated())
	{
		return;
	}

	CollectDirectionalLightData();
	CollectPointLightData();
	CollectSpotLightData();
	UploadLightDataToGPU();
}

void LightManager::MarkLightsDirty()
{
	lightDataDirty_ = true;
}
