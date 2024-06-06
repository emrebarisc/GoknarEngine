#include "pch.h"

#include "Scene.h"

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Helpers/SceneParser.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Renderer/Texture.h"

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
	for (auto staticMesh : staticMeshes_)
	{
		delete staticMesh;
	}

	for (auto dynamicMesh : dynamicMeshes_)
	{
		delete dynamicMesh;
	}

	for (auto skeletalMesh : skeletalMeshes_)
	{
		delete skeletalMesh;
	}

	for (auto texture : textures_)
	{
		delete texture;
	}

	for (auto pointLight : pointLights_)
	{
		delete pointLight;
	}

	for (auto directionalLight : directionalLights_)
	{
		delete directionalLight;
	}

	for (auto spotLight : spotLights_)
	{
		delete spotLight;
	}

	for (auto object : objects_)
	{
		delete object;
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

// TODO: TEST
void Scene::DeleteStaticMesh(StaticMesh* staticMesh)
{
	bool found = false;

	size_t staticMeshCount = staticMeshes_.size();
	for (size_t staticMeshIndex = 0; staticMeshIndex < staticMeshCount; staticMeshIndex++)
	{
		if (staticMeshes_[staticMeshIndex] == staticMesh)
		{
			staticMeshes_.erase(staticMeshes_.begin() + staticMeshIndex);
			found = true;
			break;
		}
	}

	GOKNAR_ASSERT(!found, "Destroyed StaticMesh is not found in the scene!");
}

// TODO: TEST
void Scene::DeleteSkeletalMesh(SkeletalMesh* skeletalMesh)
{
	bool found = false;

	size_t skeletalMeshCount = skeletalMeshes_.size();
	for (size_t skeletalMeshIndex = 0; skeletalMeshIndex < skeletalMeshCount; skeletalMeshIndex++)
	{
		if (skeletalMeshes_[skeletalMeshIndex] == skeletalMesh)
		{
			skeletalMeshes_.erase(skeletalMeshes_.begin() + skeletalMeshIndex);
			found = true;
			break;
		}
	}

	GOKNAR_ASSERT(!found, "Destroyed SkeletalMesh is not found in the scene!");
}

// TODO: TEST
void Scene::DeleteDynamicMesh(DynamicMesh* dynamicMesh)
{
	bool found = false;

	size_t dynamicMeshCount = dynamicMeshes_.size();
	for (size_t dynamicMeshIndex = 0; dynamicMeshIndex < dynamicMeshCount; dynamicMeshIndex++)
	{
		if (dynamicMeshes_[dynamicMeshIndex] == dynamicMesh)
		{
			dynamicMeshes_.erase(dynamicMeshes_.begin() + dynamicMeshIndex);
			found = true;
			break;
		}
	}

	GOKNAR_ASSERT(!found, "Destroyed StaticMesh is not found in the scene!");
}

void Scene::AddPointLight(PointLight* pointLight)
{
	pointLights_.push_back(pointLight);
}

void Scene::AddDirectionalLight(DirectionalLight* directionalLight)
{
	directionalLights_.push_back(directionalLight);
}

void Scene::AddSpotLight(SpotLight* spotLight)
{
	spotLights_.push_back(spotLight);
}