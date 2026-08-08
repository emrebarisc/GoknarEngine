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
	if (directionalLightUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(directionalLightUniformBufferId_);
	}

	if (directionalLightViewMatrixUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(directionalLightViewMatrixUniformBufferId_);
	}

	if (pointLightUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(pointLightUniformBufferId_);
	}

	if (spotLightUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(spotLightUniformBufferId_);
	}

	if (spotLightViewMatrixUniformBufferId_)
	{
		engine->GetGraphicsAPI()->DeleteBuffer(spotLightViewMatrixUniformBufferId_);
	}
}

void LightManager::PreInit()
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	{
		directionalLightUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(DirectionalLightBufferInfo), NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX, directionalLightUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

		directionalLightViewMatrixUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightViewMatrixUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * MAX_DIRECTIONAL_LIGHT_COUNT, NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, directionalLightViewMatrixUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);
	}

	{
		pointLightUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, pointLightUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(PointLightBufferInfo), NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, POINT_LIGHT_UNIFORM_BIND_INDEX, pointLightUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);
	}

	{
		spotLightUniformBufferId_ = graphicsAPI->CreateBuffer();
		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, spotLightUniformBufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::UniformBuffer, sizeof(SpotLightBufferInfo), NULL, GraphicsBufferUsage::DynamicDraw);

		graphicsAPI->BindBufferBase(GraphicsBufferTarget::UniformBuffer, SPOT_LIGHT_UNIFORM_BIND_INDEX, spotLightUniformBufferId_);

		graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

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
	UpdateAllDirectionalLightDataOnGPU();
	UpdateAllPointLightDataOnGPU();
	UpdateAllSpotLightDataOnGPU();

	isInitialized_ = true;
}

void LightManager::UpdateAllDirectionalLightDataOnGPU()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> directionalLights = mainScene->GetDirectionalLights();

	int directionalLightIndex = 0;
	std::vector<DirectionalLight*>::const_iterator directionalLightIterator = directionalLights.cbegin();
	while (directionalLightIndex < MAX_DIRECTIONAL_LIGHT_COUNT && directionalLightIterator != directionalLights.cend())
	{
		DirectionalLight* directionalLight = directionalLights[directionalLightIndex];

		if (directionalLight->GetIsActive())
		{
			directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].direction = directionalLight->GetDirection();
			directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].intensity = directionalLight->GetIntensity() * directionalLight->GetColor();
			directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].isCastingShadow = directionalLight->GetIsShadowEnabled();
			directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].shadowIntensity = directionalLight->GetShadowIntensity();

			directionalLight->SetUniformBufferIndex(directionalLightIndex);
			++directionalLightIndex;
		}

		++directionalLightIterator;
	}
	directionalLightBufferInfo.directionalLightCount = directionalLightIndex;

	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightUniformBufferId_);
	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, 0, sizeof(directionalLightBufferInfo.directionalLightInfo), &directionalLightBufferInfo);

	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(directionalLightBufferInfo.directionalLightInfo), sizeof(int), &directionalLightBufferInfo.directionalLightCount);
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::InitializeDirectionalLights");
}

void LightManager::UpdateAllPointLightDataOnGPU()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();

	int pointLightIndex = 0;
	std::vector<PointLight*>::const_iterator pointLightIterator = pointLights.cbegin();
	while (pointLightIndex < MAX_POINT_LIGHT_COUNT && pointLightIterator != pointLights.cend())
	{
		PointLight* pointLight = pointLights[pointLightIndex];

		if (pointLight->GetIsActive())
		{
			pointLightBufferInfo.pointLightInfo[pointLightIndex].position = pointLight->GetPosition();
			pointLightBufferInfo.pointLightInfo[pointLightIndex].radius = pointLight->GetRadius();
			pointLightBufferInfo.pointLightInfo[pointLightIndex].intensity = pointLight->GetIntensity() * pointLight->GetColor();
			pointLightBufferInfo.pointLightInfo[pointLightIndex].isCastingShadow = pointLight->GetIsShadowEnabled();
			pointLightBufferInfo.pointLightInfo[pointLightIndex].shadowIntensity = pointLight->GetShadowIntensity();

			pointLight->SetUniformBufferIndex(pointLightIndex);
			++pointLightIndex;
		}

		++pointLightIterator;
	}
	pointLightBufferInfo.pointLightCount = pointLightIndex;

	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, pointLightUniformBufferId_);
	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, 0, sizeof(pointLightBufferInfo.pointLightInfo), &pointLightBufferInfo);

	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(pointLightBufferInfo.pointLightInfo), sizeof(int), &pointLightBufferInfo.pointLightCount);
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::InitializePointLights");
}

void LightManager::UpdateAllSpotLightDataOnGPU()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();

	int spotLightIndex = 0;
	std::vector<SpotLight*>::const_iterator spotLightIterator = spotLights.cbegin();
	while (spotLightIndex < MAX_SPOT_LIGHT_COUNT && spotLightIterator != spotLights.cend())
	{
		SpotLight* spotLight = spotLights[spotLightIndex];

		if (spotLight->GetIsActive())
		{

			spotLightBufferInfo.spotLightInfo[spotLightIndex].position = spotLight->GetPosition();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].coverageAngle = spotLight->GetCoverageAngle();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].direction = spotLight->GetDirection();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].falloffAngle = spotLight->GetFalloffAngle();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].intensity = spotLight->GetIntensity() * spotLight->GetColor();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].isCastingShadow = spotLight->GetIsShadowEnabled();
			spotLightBufferInfo.spotLightInfo[spotLightIndex].shadowIntensity = spotLight->GetShadowIntensity();

			spotLight->SetUniformBufferIndex(spotLightIndex);
			++spotLightIndex;
		}

		++spotLightIterator;
	}
	spotLightBufferInfo.spotLightCount = spotLightIndex;

	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, spotLightUniformBufferId_);
	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, 0, sizeof(spotLightBufferInfo.spotLightInfo), &spotLightBufferInfo);

	graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(spotLightBufferInfo.spotLightInfo), sizeof(int), &spotLightBufferInfo.spotLightCount);
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::InitializeSpotLights");
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

	UpdateAllDirectionalLightDataOnGPU();
	UpdateAllPointLightDataOnGPU();
	UpdateAllSpotLightDataOnGPU();

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
}

void LightManager::BindShadowViewProjectionMatrices()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> directionalLights = mainScene->GetDirectionalLights();
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, directionalLightViewMatrixUniformBufferId_);
	for (int directionalLightIndex = 0; directionalLightIndex < directionalLights.size(); ++directionalLightIndex)
	{
		graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * directionalLightIndex, sizeof(Matrix), &directionalLights[directionalLightIndex]->GetBiasedShadowViewProjectionMatrix());
	}
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, spotLightViewMatrixUniformBufferId_);
	for (int spotLightIndex = 0; spotLightIndex < spotLights.size(); ++spotLightIndex)
	{
		graphicsAPI->BufferSubData(GraphicsBufferTarget::UniformBuffer, sizeof(Matrix) * spotLightIndex, sizeof(Matrix), &spotLights[spotLightIndex]->GetBiasedShadowViewProjectionMatrix());
	}
	graphicsAPI->BindBuffer(GraphicsBufferTarget::UniformBuffer, 0);

	EXIT_ON_GRAPHICS_API_ERROR("LightManager::BindShadowViewProjectionMatrices");
}

void LightManager::OnDirectionalLightAdded(DirectionalLight* directionalLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllDirectionalLightDataOnGPU();
}

void LightManager::OnDirectionalLightRemoved(DirectionalLight* directionalLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllDirectionalLightDataOnGPU();
}

void LightManager::OnPointLightAdded(PointLight* pointLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllPointLightDataOnGPU();
}

void LightManager::OnPointLightRemoved(PointLight* pointLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllPointLightDataOnGPU();
}

void LightManager::OnSpotLightAdded(SpotLight* spotLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllSpotLightDataOnGPU();
}

void LightManager::OnSpotLightRemoved(SpotLight* spotLight)
{
	if (!isInitialized_)
	{
		return;
	}

	//TODO: Temporarily update all corresponding light data
	UpdateAllSpotLightDataOnGPU();
}

void LightManager::UpdateLights()
{
	if(!isInitialized_)
	{
		return;
	}

	UpdateAllDirectionalLightDataOnGPU();
	UpdateAllPointLightDataOnGPU();
	UpdateAllSpotLightDataOnGPU();
}
