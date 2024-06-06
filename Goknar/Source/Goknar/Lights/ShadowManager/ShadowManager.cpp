#include "pch.h"

#include "ShadowManager.h"

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

ShadowManager::ShadowManager()
{
}

ShadowManager::~ShadowManager()
{
}

void ShadowManager::PreInit()
{
	{
		glGenBuffers(1, &directionalLightUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLightBufferInfo), NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX, directionalLightUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &directionalLightViewMatrixUniformBufferId_);
		glBindBuffer(GL_UNIFORM_BUFFER, directionalLightViewMatrixUniformBufferId_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix), NULL, GL_DYNAMIC_DRAW);

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
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix), NULL, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, SPOT_LIGHT_VIEW_MATRIX_UNIFORM_BIND_INDEX, spotLightViewMatrixUniformBufferId_);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	EXIT_ON_GL_ERROR("ShadowManager::PreInit");
}

void ShadowManager::Init()
{
}

void ShadowManager::PostInit()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetDirectionalLights();

	int directionalLightIndex = 0;
	std::vector<DirectionalLight*>::const_iterator directionalLightIterator = staticDirectionalLights.cbegin();
	while (directionalLightIterator != staticDirectionalLights.cend())
	{
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].direction = staticDirectionalLights[directionalLightIndex]->GetDirection();
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].intensity = staticDirectionalLights[directionalLightIndex]->GetIntensity();
		directionalLightBufferInfo.directionalLightInfo[directionalLightIndex].isCastingShadow = staticDirectionalLights[directionalLightIndex]->GetIsShadowEnabled();

		staticDirectionalLights[directionalLightIndex]->SetUniformBufferIndex(directionalLightIndex);

		++directionalLightIndex;
		++directionalLightIterator;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(directionalLightBufferInfo.directionalLightInfo), &directionalLightBufferInfo);

	int directionalLightCount = 1;
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(directionalLightBufferInfo.directionalLightInfo), sizeof(int), &directionalLightCount);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("GL_UNIFORM_BUFFER");
}

void ShadowManager::RenderShadowMaps()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	// Only draw the depth buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const std::vector<PointLight*> pointLights = mainScene->GetPointLights();
	size_t staticPointLightCount = pointLights.size();
	for (size_t i = 0; i < staticPointLightCount; i++)
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
		size_t staticDirectionalLightCount = directionalLights.size();
		for (size_t i = 0; i < staticDirectionalLightCount; i++)
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
	size_t staticSpotLightCount = spotLights.size();
	for (size_t i = 0; i < staticSpotLightCount; i++)
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

	EXIT_ON_GL_ERROR("ShadowManager::RenderShadowMaps");
}

void ShadowManager::SetShadowRenderPassShaderUniforms(const Shader* shader) const
{
	if (!currentlyRenderingLight_)
	{
		return;
	}

	currentlyRenderingLight_->SetShadowRenderPassShaderUniforms(shader);
}

void ShadowManager::BindLightUniforms(Shader* shader)
{
}

void ShadowManager::BindShadowViewProjectionMatrices()
{
	EXIT_ON_GL_ERROR("ShadowManager::BindShadowViewProjectionMatrices");
	Scene* mainScene = engine->GetApplication()->GetMainScene();
	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetDirectionalLights();
	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightViewMatrixUniformBufferId_);
	for (int directionalLightIndex = 0; directionalLightIndex < staticDirectionalLights.size(); ++directionalLightIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix) * directionalLightIndex, sizeof(Matrix), &staticDirectionalLights[directionalLightIndex]->GetBiasedShadowViewProjectionMatrix());
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("ShadowManager::BindShadowViewProjectionMatrices");
}
