#include "pch.h"

#include "ShadowManager.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"

#include "Goknar/IO/IOManager.h"
#include "Goknar/Renderer/Texture.h"

void ShadowManager::RenderShadowMaps()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	CameraManager* cameraManager = engine->GetCameraManager();
	Camera* mainCamera = engine->GetCameraManager()->GetActiveCamera();

	EXIT_ON_GL_ERROR("ShadowManager::RenderShadowMaps 0 ");
	// Only draw the depth buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetStaticDirectionalLights();
	size_t staticDirectionalLightCount = staticDirectionalLights.size();
	for (size_t i = 0; i < staticDirectionalLightCount; i++)
	{
		DirectionalLight* directionalLight = staticDirectionalLights[i];
		if (!directionalLight->GetIsShadowEnabled())
		{
			continue;
		}

		directionalLight->RenderShadowMap();
	}

	const std::vector<PointLight*> staticPointLights = mainScene->GetStaticPointLights();
	size_t staticPointLightCount = staticPointLights.size();
	for (size_t i = 0; i < staticPointLightCount; i++)
	{
		PointLight* pointLight = staticPointLights[i];
		if (!pointLight->GetIsShadowEnabled())
		{
			continue;
		}

		pointLight->RenderShadowMap();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	cameraManager->SetActiveCamera(mainCamera);

	EXIT_ON_GL_ERROR("ShadowManager::RenderShadowMaps");
}
