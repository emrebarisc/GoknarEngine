#include "pch.h"

#include "LightManager.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"

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
}

void LightManager::PreInit()
{
	{
		glGenBuffers(1, &directionalLightUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLightBufferInfo), NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX, directionalLightUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &directionalLightViewMatrixUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, directionalLightViewMatrixUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix) * MAX_DIRECTIONAL_LIGHT_COUNT, NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, directionalLightViewMatrixUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	{
		glGenBuffers(1, &pointLightUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, pointLightUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLightBufferInfo), NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, POINT_LIGHT_UNIFORM_BIND_INDEX, pointLightUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	{
		glGenBuffers(1, &spotLightUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, spotLightUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SpotLightBufferInfo), NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, SPOT_LIGHT_UNIFORM_BIND_INDEX, spotLightUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &spotLightViewMatrixUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, spotLightViewMatrixUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix) * MAX_SPOT_LIGHT_COUNT, NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, spotLightViewMatrixUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	EXIT_ON_GL_ERROR("LightManager::PreInit");
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
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].direction = directionalLights[directionalLightIndex]->GetDirection();
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].intensity = directionalLights[directionalLightIndex]->GetIntensity() * directionalLights[directionalLightIndex]->GetColor();
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].isCastingShadow = directionalLights[directionalLightIndex]->GetIsShadowEnabled();
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].shadowIntensity = directionalLights[directionalLightIndex]->GetShadowIntensity();

		directionalLights[directionalLightIndex]->SetUniformBufferIndex(directionalLightIndex);

		++directionalLightIndex;
		++directionalLightIterator;
	}
	directionalLightBufferInfo.directionalLightCount = directionalLightIndex;

	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(directionalLightBufferInfo.directionalLightInfo), &directionalLightBufferInfo);

	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(directionalLightBufferInfo.directionalLightInfo), sizeof(int), &directionalLightBufferInfo.directionalLightCount);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("LightManager::InitializeDirectionalLights");
}

void LightManager::UpdateAllPointLightDataOnGPU()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();

	int pointLightIndex = 0;
	std::vector<PointLight*>::const_iterator pointLightIterator = pointLights.cbegin();
	while (pointLightIndex < MAX_POINT_LIGHT_COUNT && pointLightIterator != pointLights.cend())
	{
		pointLightBufferInfo.pointLightInfo[pointLightIndex].position = pointLights[pointLightIndex]->GetPosition();
		pointLightBufferInfo.pointLightInfo[pointLightIndex].radius = pointLights[pointLightIndex]->GetRadius();
		pointLightBufferInfo.pointLightInfo[pointLightIndex].intensity = pointLights[pointLightIndex]->GetIntensity() * pointLights[pointLightIndex]->GetColor();
		pointLightBufferInfo.pointLightInfo[pointLightIndex].isCastingShadow = pointLights[pointLightIndex]->GetIsShadowEnabled();
		pointLightBufferInfo.pointLightInfo[pointLightIndex].shadowIntensity = pointLights[pointLightIndex]->GetShadowIntensity();

		pointLights[pointLightIndex]->SetUniformBufferIndex(pointLightIndex);

		++pointLightIndex;
		++pointLightIterator;
	}
	pointLightBufferInfo.pointLightCount = pointLightIndex;

	glBindBuffer(GL_UNIFORM_BUFFER, pointLightUniformBufferId_);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(pointLightBufferInfo.pointLightInfo), &pointLightBufferInfo);

	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(pointLightBufferInfo.pointLightInfo), sizeof(int), &pointLightBufferInfo.pointLightCount);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("LightManager::InitializePointLights");
}

void LightManager::UpdateAllSpotLightDataOnGPU()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();

	int spotLightIndex = 0;
	std::vector<SpotLight*>::const_iterator spotLightIterator = spotLights.cbegin();
	while (spotLightIndex < MAX_SPOT_LIGHT_COUNT && spotLightIterator != spotLights.cend())
	{
		spotLightBufferInfo.spotLightInfo[spotLightIndex].position = spotLights[spotLightIndex]->GetPosition();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].coverageAngle = spotLights[spotLightIndex]->GetCoverageAngle();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].direction = spotLights[spotLightIndex]->GetDirection();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].falloffAngle = spotLights[spotLightIndex]->GetFalloffAngle();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].intensity = spotLights[spotLightIndex]->GetIntensity() * spotLights[spotLightIndex]->GetColor();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].isCastingShadow = spotLights[spotLightIndex]->GetIsShadowEnabled();
		spotLightBufferInfo.spotLightInfo[spotLightIndex].shadowIntensity = spotLights[spotLightIndex]->GetShadowIntensity();

		spotLights[spotLightIndex]->SetUniformBufferIndex(spotLightIndex);

		++spotLightIndex;
		++spotLightIterator;
	}
	spotLightBufferInfo.spotLightCount = spotLightIndex;

	glBindBuffer(GL_UNIFORM_BUFFER, spotLightUniformBufferId_);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(spotLightBufferInfo.spotLightInfo), &spotLightBufferInfo);

	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(spotLightBufferInfo.spotLightInfo), sizeof(int), &spotLightBufferInfo.spotLightCount);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("LightManager::InitializeSpotLights");
}

void LightManager::RenderShadowMaps()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	// Only draw the depth buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();
	size_t pointLightCount = pointLights.size();
	for (size_t i = 0; i < pointLightCount; i++)
	{
		PointLight* pointLight = pointLights[i];
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
			cameraManager->SetActiveCamera(mainCamera);

			DirectionalLight* directionalLight = directionalLights[i];
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
		if (!spotLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = spotLight;
		spotLight->RenderShadowMap();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	cameraManager->SetActiveCamera(mainCamera);

	BindShadowViewProjectionMatrices();

	UpdateAllDirectionalLightDataOnGPU();
	UpdateAllPointLightDataOnGPU();
	UpdateAllSpotLightDataOnGPU();

	EXIT_ON_GL_ERROR("LightManager::RenderShadowMaps");
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
	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightViewMatrixUniformBufferId_);
	for (int directionalLightIndex = 0; directionalLightIndex < directionalLights.size(); ++directionalLightIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix) * directionalLightIndex, sizeof(Matrix), &directionalLights[directionalLightIndex]->GetBiasedShadowViewProjectionMatrix());
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	const std::vector<SpotLight*> spotLights = mainScene->GetSpotLights();
	glBindBuffer(GL_UNIFORM_BUFFER, spotLightViewMatrixUniformBufferId_);
	for (int spotLightIndex = 0; spotLightIndex < spotLights.size(); ++spotLightIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix) * spotLightIndex, sizeof(Matrix), &spotLights[spotLightIndex]->GetBiasedShadowViewProjectionMatrix());
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("LightManager::BindShadowViewProjectionMatrices");
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
