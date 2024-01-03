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

#include "Goknar/TimeDependentObject.h"

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

	for (auto material : materials_)
	{
		delete material;
	}

	for (auto texture : textures_)
	{
		delete texture;
	}

	for (auto staticPointLight : staticPointLights_)
	{
		delete staticPointLight;
	}

	for (auto staticDirectionalLight : staticDirectionalLights_)
	{
		delete staticDirectionalLight;
	}

	for (auto staticSpotLight : staticSpotLights_)
	{
		delete staticSpotLight;
	}

	for (auto dynamicDirectionalLight : dynamicDirectionalLights_)
	{
		delete dynamicDirectionalLight;
	}

	for (auto dynamicPointLight : dynamicPointLights_)
	{
		delete dynamicPointLight;
	}

	for (auto dynamicSpotLight : dynamicSpotLights_)
	{
		delete dynamicSpotLight;
	}

	for (auto object : objects_)
	{
		delete object;
	}
}

void Scene::PreInit()
{
	for (auto staticPointLight : staticPointLights_)
	{
		staticPointLight->PreInit();
	}

	for (auto staticDirectionalLight : staticDirectionalLights_)
	{
		staticDirectionalLight->PreInit();
	}

	for (auto staticSpotLight : staticSpotLights_)
	{
		staticSpotLight->PreInit();
	}

	for (auto dynamicDirectionalLight : dynamicDirectionalLights_)
	{
		dynamicDirectionalLight->PreInit();
	}

	for (auto dynamicPointLight : dynamicPointLights_)
	{
		dynamicPointLight->PreInit();
	}

	for (auto dynamicSpotLight : dynamicSpotLights_)
	{
		dynamicSpotLight->PreInit();
	}

	for (Texture* texture : textures_)
	{
		texture->PreInit();
	}
}

void Scene::Init()
{
	for (auto staticPointLight : staticPointLights_)
	{
		staticPointLight->Init();
	}

	for (auto staticDirectionalLight : staticDirectionalLights_)
	{
		staticDirectionalLight->Init();
	}

	for (auto staticSpotLight : staticSpotLights_)
	{
		staticSpotLight->Init();
	}

	for (auto dynamicDirectionalLight : dynamicDirectionalLights_)
	{
		dynamicDirectionalLight->Init();
	}

	for (auto dynamicPointLight : dynamicPointLights_)
	{
		dynamicPointLight->Init();
	}

	for (auto dynamicSpotLight : dynamicSpotLights_)
	{
		dynamicSpotLight->Init();
	}

	for (Texture* texture : textures_)
	{
		texture->Init();
	}
}

void Scene::PostInit()
{
	for (auto staticPointLight : staticPointLights_)
	{
		staticPointLight->PostInit();
	}

	for (auto staticDirectionalLight : staticDirectionalLights_)
	{
		staticDirectionalLight->PostInit();
	}

	for (auto staticSpotLight : staticSpotLights_)
	{
		staticSpotLight->PostInit();
	}

	for (auto dynamicDirectionalLight : dynamicDirectionalLights_)
	{
		dynamicDirectionalLight->PostInit();
	}

	for (auto dynamicPointLight : dynamicPointLights_)
	{
		dynamicPointLight->PostInit();
	}

	for (auto dynamicSpotLight : dynamicSpotLights_)
	{
		dynamicSpotLight->PostInit();
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
	switch (pointLight->GetLightMobility())
	{
	case LightMobility::Static:
		staticPointLights_.push_back(pointLight);
		break;
	case LightMobility::Dynamic:
		dynamicPointLights_.push_back(pointLight);
		break;
	}

}

void Scene::AddDirectionalLight(DirectionalLight* directionalLight)
{
	switch (directionalLight->GetLightMobility())
	{
	case LightMobility::Static:
		staticDirectionalLights_.push_back(directionalLight);
		break;
	case LightMobility::Dynamic:
		dynamicDirectionalLights_.push_back(directionalLight);
		break;
	}
}


void Scene::AddSpotLight(SpotLight* spotLight)
{
	switch (spotLight->GetLightMobility())
	{
	case LightMobility::Static:
		staticSpotLights_.push_back(spotLight);
		break;
	case LightMobility::Dynamic:
		dynamicSpotLights_.push_back(spotLight);
		break;
	}
}