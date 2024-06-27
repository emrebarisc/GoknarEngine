#include "pch.h"

#include "Scene.h"

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Helpers/SceneParser.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Lights/LightManager/LightManager.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"

#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

Scene::Scene()
{
	backgroundColor_ = Colori(0, 0, 0);
    ambientLight_ = Vector3::ZeroVector;
}

Scene::~Scene()
{
	for (auto texture : textures_)
	{
		delete texture;
	}
}

void Scene::PreInit()
{
	for (auto pointLight : pointLights_)
	{
		pointLight->PreInit();
	}

	for (auto directionalLight : directionalLights_)
	{
		directionalLight->PreInit();
	}

	for (auto spotLight : spotLights_)
	{
		spotLight->PreInit();
	}

	for (Texture* texture : textures_)
	{
		texture->PreInit();
	}
}

void Scene::Init()
{
	for (auto pointLight : pointLights_)
	{
		pointLight->Init();
	}

	for (auto directionalLight : directionalLights_)
	{
		directionalLight->Init();
	}

	for (auto spotLight : spotLights_)
	{
		spotLight->Init();
	}

	for (Texture* texture : textures_)
	{
		texture->Init();
	}
}

void Scene::PostInit()
{
	for (auto pointLight : pointLights_)
	{
		pointLight->PostInit();
	}

	for (auto directionalLight : directionalLights_)
	{
		directionalLight->PostInit();
	}

	for (auto spotLight : spotLights_)
	{
		spotLight->PostInit();
	}

	for (Texture* texture : textures_)
	{
		texture->PostInit();
	}
}

void Scene::ReadSceneData(const std::string& filePath)
{
    SceneParser::Parse(this, ContentDir + filePath);
}

void Scene::AddDirectionalLight(DirectionalLight* directionalLight)
{
	directionalLights_.push_back(directionalLight);

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnDirectionalLightAdded(directionalLight);
	}
}

void Scene::RemoveDirectionalLight(DirectionalLight* directionalLight)
{
	std::vector<DirectionalLight*>::const_iterator directionalLightIterator = directionalLights_.cbegin();
	while (directionalLightIterator != directionalLights_.cend())
	{
		if (*directionalLightIterator == directionalLight)
		{
			directionalLights_.erase(directionalLightIterator);
			break;
		}

		++directionalLightIterator;
	}

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnDirectionalLightRemoved(directionalLight);
	}
}

void Scene::AddPointLight(PointLight* pointLight)
{
	pointLights_.push_back(pointLight);

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnPointLightAdded(pointLight);
	}
}

void Scene::RemovePointLight(PointLight* pointLight)
{
	std::vector<PointLight*>::const_iterator pointLightIterator = pointLights_.cbegin();
	while (pointLightIterator != pointLights_.cend())
	{
		if (*pointLightIterator == pointLight)
		{
			pointLights_.erase(pointLightIterator);
			break;
		}

		++pointLightIterator;
	}

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnPointLightRemoved(pointLight);
	}
}

void Scene::AddSpotLight(SpotLight* spotLight)
{
	spotLights_.push_back(spotLight);

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnSpotLightAdded(spotLight);
	}
}

void Scene::RemoveSpotLight(SpotLight* spotLight)
{
	std::vector<SpotLight*>::const_iterator spotLightIterator = spotLights_.cbegin();
	while (spotLightIterator != spotLights_.cend())
	{
		if (*spotLightIterator == spotLight)
		{
			spotLights_.erase(spotLightIterator);
			break;
		}

		++spotLightIterator;
	}

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager)
	{
		lightManager->OnSpotLightRemoved(spotLight);
	}
}