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
	glGenBuffers(1, &directionalLightUniformBufferId_);
	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLightBufferInfo), NULL, GL_STATIC_DRAW);

	//glBindBufferRange(GL_UNIFORM_BUFFER, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX, directionalLightUniformBufferId_, 0, 32);
	glBindBufferBase(GL_UNIFORM_BUFFER, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX, directionalLightUniformBufferId_);

	//glGenBuffers(1, &pointLightUniformBufferId_);
	//glBindBuffer(GL_UNIFORM_BUFFER, pointLightUniformBufferId_);
	////glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW); // allocate 152 bytes of memory
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//glGenBuffers(1, &spotLightUniformBufferId_);
	//glBindBuffer(GL_UNIFORM_BUFFER, spotLightUniformBufferId_);
	////glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW); // allocate 152 bytes of memory
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	EXIT_ON_GL_ERROR("ShadowManager::PreInit");
}

void ShadowManager::Init()
{
}

void ShadowManager::PostInit()
{
	Scene* mainScene = engine->GetApplication()->GetMainScene();

	const std::vector<DirectionalLight*> staticDirectionalLights = mainScene->GetStaticDirectionalLights();

	directionalLightBufferInfo.directionalLightInfo[0].direction = staticDirectionalLights[0]->GetDirection();
	directionalLightBufferInfo.directionalLightInfo[0].intensity = staticDirectionalLights[0]->GetIntensity();
	directionalLightBufferInfo.directionalLightInfo[0].isCastingShadow = staticDirectionalLights[0]->GetIsShadowEnabled();

	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(directionalLightBufferInfo.directionalLightInfo), &directionalLightBufferInfo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, directionalLightUniformBufferId_);
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

	if(mainCamera)
	{
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

void ShadowManager::BindLightUniforms(Shader* shader)
{
	shader->Use();
	unsigned int shaderId = shader->GetProgramId();

	unsigned int uniformBlockIndexDirectionalLights = glGetUniformBlockIndex(shaderId, SHADER_VARIABLE_NAMES::LIGHT::DIRECTIONAL_LIGHT_UNIFORM_NAME);
	glUniformBlockBinding(shaderId, uniformBlockIndexDirectionalLights, DIRECTIONAL_LIGHT_UNIFORM_BIND_INDEX);

	unsigned int uniformBlockIndexPointLights = glGetUniformBlockIndex(shaderId, SHADER_VARIABLE_NAMES::LIGHT::POINT_LIGHT_UNIFORM_NAME);
	glUniformBlockBinding(shaderId, uniformBlockIndexPointLights, POINT_LIGHT_UNIFORM_BIND_INDEX);

	unsigned int uniformBlockIndexSpotLights = glGetUniformBlockIndex(shaderId, SHADER_VARIABLE_NAMES::LIGHT::SPOT_LIGHT_UNIFORM_NAME);
	glUniformBlockBinding(shaderId, uniformBlockIndexSpotLights, SPOT_LIGHT_UNIFORM_BIND_INDEX);
	EXIT_ON_GL_ERROR("ShadowManager::BindLightUniforms");

	shader->Unbind();
}
