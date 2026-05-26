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
#include "Goknar/Navigation/NavigationMesh.h"
#include "Goknar/Navigation/NavigationTypes.h"
#include "Goknar/Renderer/ReflectionProbe.h"

Scene::Scene()
{
	backgroundColor_ = Colori(0, 0, 0);
    ambientLight_ = Vector3::ZeroVector;
	navigationMesh_ = new NavigationMesh();
}

Scene::~Scene()
{
	while (0 < pointLights_.size())
	{
		delete pointLights_[0];
	}

	while (0 < directionalLights_.size())
	{
		delete directionalLights_[0];
	}

	while (0 < spotLights_.size())
	{
		delete spotLights_[0];
	}

	while (0 < reflectionProbes_.size())
	{
		delete reflectionProbes_[0];
	}

	for (auto texture : textures_)
	{
		delete texture;
	}

	delete navigationMesh_;
	navigationMesh_ = nullptr;
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

	for (auto reflectionProbe : reflectionProbes_)
	{
		reflectionProbe->PreInit();
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

	for (auto reflectionProbe : reflectionProbes_)
	{
		reflectionProbe->Init();
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

	for (auto reflectionProbe : reflectionProbes_)
	{
		reflectionProbe->PostInit();
	}

	for (Texture* texture : textures_)
	{
		texture->PostInit();
	}
}

void Scene::ReadSceneData(const std::string& filePath)
{
	path_ = filePath;
    SceneParser::Parse(this, ContentDir + filePath);

	const NavMeshSettings navMeshSettings;
#ifdef GOKNAR_EDITOR
	constexpr bool removeNavigationTreeObjects = false;
#else
	constexpr bool removeNavigationTreeObjects = true;
#endif
	RebuildNavigationMesh(navMeshSettings, removeNavigationTreeObjects);
}

void Scene::RebuildNavigationMesh(const NavMeshSettings& settings, bool removeNavigationTreeObjects)
{
	if (navigationMesh_)
	{
		navigationMesh_->BuildFromScene(this, settings, removeNavigationTreeObjects);
	}
}

void Scene::AddObject(ObjectBase* object, bool isFromReferencedScene)
{
	if (!object)
	{
		return;
	}

	if (objectReferencedSceneState_.find(object) == objectReferencedSceneState_.end())
	{
		objects_.push_back(object);
	}

	objectReferencedSceneState_[object] = isFromReferencedScene;
}

void Scene::RemoveObject(ObjectBase* object)
{
	if (!object)
	{
		return;
	}

	const std::vector<ObjectBase*> childObjects = object->GetChildren();
	for (ObjectBase* childObject : childObjects)
	{
		RemoveObject(childObject);
	}

	auto objectIterator = objects_.begin();
	while (objectIterator != objects_.end())
	{
		if (*objectIterator == object)
		{
			objects_.erase(objectIterator);
			break;
		}

		++objectIterator;
	}

	objectReferencedSceneState_.erase(object);

	auto sceneReferenceIterator = sceneReferences_.begin();
	while (sceneReferenceIterator != sceneReferences_.end())
	{
		if (sceneReferenceIterator->sceneRootObject == object)
		{
			sceneReferenceIterator = sceneReferences_.erase(sceneReferenceIterator);
			continue;
		}

		++sceneReferenceIterator;
	}
}

void Scene::DestroyObjects()
{
	std::vector<ObjectBase*> objectsToDestroy = objects_;
	objects_.clear();
	objectReferencedSceneState_.clear();
	sceneReferences_.clear();

	for (ObjectBase* object : objectsToDestroy)
	{
		if (object)
		{
			object->Destroy();
		}
	}
}

bool Scene::GetIsObjectFromReferencedScene(ObjectBase* object) const
{
	auto objectIterator = objectReferencedSceneState_.find(object);
	return objectIterator != objectReferencedSceneState_.end() && objectIterator->second;
}

void Scene::AddSceneReference(const SceneReference& sceneReference)
{
	if (sceneReference.path.empty())
	{
		return;
	}

	sceneReferences_.push_back(sceneReference);
}

void Scene::AddDirectionalLight(DirectionalLight* directionalLight, bool isFromReferencedScene)
{
	if (!directionalLight)
	{
		return;
	}

	const bool isNewDirectionalLight = directionalLightReferencedSceneState_.find(directionalLight) == directionalLightReferencedSceneState_.end();
	if (isNewDirectionalLight)
	{
		directionalLights_.push_back(directionalLight);
	}

	directionalLightReferencedSceneState_[directionalLight] = isFromReferencedScene;

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager && isNewDirectionalLight)
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

	directionalLightReferencedSceneState_.erase(directionalLight);
}

bool Scene::GetIsDirectionalLightFromReferencedScene(DirectionalLight* directionalLight) const
{
	auto directionalLightIterator = directionalLightReferencedSceneState_.find(directionalLight);
	return directionalLightIterator != directionalLightReferencedSceneState_.end() && directionalLightIterator->second;
}

void Scene::AddPointLight(PointLight* pointLight, bool isFromReferencedScene)
{
	if (!pointLight)
	{
		return;
	}

	const bool isNewPointLight = pointLightReferencedSceneState_.find(pointLight) == pointLightReferencedSceneState_.end();
	if (isNewPointLight)
	{
		pointLights_.push_back(pointLight);
	}

	pointLightReferencedSceneState_[pointLight] = isFromReferencedScene;

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager && isNewPointLight)
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

	pointLightReferencedSceneState_.erase(pointLight);
}

bool Scene::GetIsPointLightFromReferencedScene(PointLight* pointLight) const
{
	auto pointLightIterator = pointLightReferencedSceneState_.find(pointLight);
	return pointLightIterator != pointLightReferencedSceneState_.end() && pointLightIterator->second;
}

void Scene::AddSpotLight(SpotLight* spotLight, bool isFromReferencedScene)
{
	if (!spotLight)
	{
		return;
	}

	const bool isNewSpotLight = spotLightReferencedSceneState_.find(spotLight) == spotLightReferencedSceneState_.end();
	if (isNewSpotLight)
	{
		spotLights_.push_back(spotLight);
	}

	spotLightReferencedSceneState_[spotLight] = isFromReferencedScene;

	LightManager* lightManager = engine->GetRenderer()->GetLightManager();
	if (lightManager && isNewSpotLight)
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

	spotLightReferencedSceneState_.erase(spotLight);
}

bool Scene::GetIsSpotLightFromReferencedScene(SpotLight* spotLight) const
{
	auto spotLightIterator = spotLightReferencedSceneState_.find(spotLight);
	return spotLightIterator != spotLightReferencedSceneState_.end() && spotLightIterator->second;
}

void Scene::AddReflectionProbe(ReflectionProbe* reflectionProbe)
{
	reflectionProbes_.push_back(reflectionProbe);
}

void Scene::RemoveReflectionProbe(ReflectionProbe* reflectionProbe)
{
	std::vector<ReflectionProbe*>::const_iterator reflectionProbeIterator = reflectionProbes_.cbegin();
	while (reflectionProbeIterator != reflectionProbes_.cend())
	{
		if (*reflectionProbeIterator == reflectionProbe)
		{
			reflectionProbes_.erase(reflectionProbeIterator);
			break;
		}

		++reflectionProbeIterator;
	}
}
