#include "pch.h"

#include "ShadowManager.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Materials/Material.h"
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

void ShadowManager::RenderShadowMaps()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	// Only draw the depth buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const std::vector<PointLight*> staticPointLights = mainScene->GetStaticPointLights();
	size_t staticPointLightCount = staticPointLights.size();
	for (size_t i = 0; i < staticPointLightCount; i++)
	{
		PointLight* pointLight = staticPointLights[i];
		if (!pointLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = pointLight;
		pointLight->RenderShadowMap();
	}

	const std::vector<PointLight*> dynamicPointLights = mainScene->GetDynamicPointLights();
	size_t dynamicPointLightCount = dynamicPointLights.size();
	for (size_t i = 0; i < dynamicPointLightCount; i++)
	{
		PointLight* pointLight = dynamicPointLights[i];
		if (!pointLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = pointLight;
		pointLight->RenderShadowMap();
	}

	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetStaticDirectionalLights();
	size_t staticDirectionalLightCount = staticDirectionalLights.size();
	for (size_t i = 0; i < staticDirectionalLightCount; i++)
	{
		cameraManager->SetActiveCamera(mainCamera);

		DirectionalLight* directionalLight = staticDirectionalLights[i];
		if (!directionalLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = directionalLight;
		directionalLight->RenderShadowMap();
	}

	const std::vector<DirectionalLight*> dynamicDirectionalLights = mainScene->GetDynamicDirectionalLights();
	size_t dynamicDirectionalLightCount = dynamicDirectionalLights.size();
	for (size_t i = 0; i < dynamicDirectionalLightCount; i++)
	{
		cameraManager->SetActiveCamera(mainCamera);

		DirectionalLight* directionalLight = dynamicDirectionalLights[i];
		if (!directionalLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = directionalLight;
		directionalLight->RenderShadowMap();
	}

	const std::vector<SpotLight*> staticSpotLights = mainScene->GetStaticSpotLights();
	size_t staticSpotLightCount = staticSpotLights.size();
	for (size_t i = 0; i < staticSpotLightCount; i++)
	{
		SpotLight* spotLight = staticSpotLights[i];
		if (!spotLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = spotLight;
		spotLight->RenderShadowMap();
	}

	const std::vector<SpotLight*> dynamicSpotLights = mainScene->GetDynamicSpotLights();
	size_t dynamicSpotLightCount = dynamicSpotLights.size();
	for (size_t i = 0; i < dynamicSpotLightCount; i++)
	{
		SpotLight* spotLight = dynamicSpotLights[i];
		if (!spotLight->GetIsShadowEnabled())
		{
			continue;
		}

		currentlyRenderingLight_ = spotLight;
		spotLight->RenderShadowMap();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	cameraManager->SetActiveCamera(mainCamera);

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
