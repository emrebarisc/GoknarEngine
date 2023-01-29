#include "pch.h"

#include "Scene.h"

#include "Engine.h"
#include "Goknar/Log.h"
#include "Material.h"
#include "ObjectBase.h"
#include "Helpers/SceneParser.h"
#include "Renderer/Texture.h"

#include "Model/DynamicMesh.h"
#include "Model/StaticMesh.h"
#include "Model/SkeletalMesh.h"

#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"

#include "TimeDependentObject.h"

Scene::Scene()
{
	backgroundColor_ = Colori(0, 0, 0);
    ambientLight_ = Vector3::ZeroVector;
}

Scene::~Scene()
{
	for (auto object : staticMeshes_)
	{
		delete object;
	}

	for (auto object : dynamicMeshes_)
	{
		delete object;
	}

	for (auto object : shaders_)
	{
		delete object;
	}

	for (auto object : materials_)
	{
		delete object;
	}

	for (auto object : textures_)
	{
		delete object;
	}

	for (auto object : staticPointLights_)
	{
		delete object;
	}

	for (auto object : staticDirectionalLights_)
	{
		delete object;
	}

	for (auto object : staticSpotLights_)
	{
		delete object;
	}

	for (auto object : dynamicDirectionalLights_)
	{
		delete object;
	}

	for (auto object : dynamicPointLights_)
	{
		delete object;
	}

	for (auto object : dynamicSpotLights_)
	{
		delete object;
	}

	for (auto object : objects_)
	{
		delete object;
	}
}

void Scene::Init()
{
	for (auto object : staticPointLights_)
	{
		object->Init();
	}

	for (auto object : staticDirectionalLights_)
	{
		object->Init();
	}

	for (auto object : staticSpotLights_)
	{
		object->Init();
	}

	for (auto object : dynamicDirectionalLights_)
	{
		object->Init();
	}

	for (auto object : dynamicPointLights_)
	{
		object->Init();
	}

	for (auto object : dynamicSpotLights_)
	{
		object->Init();
	}

	for (Texture* texture : textures_)
	{
		texture->Init();
	}

	for (Material* material : materials_)
	{
		material->Init();
	}

	for (StaticMesh* staticMesh : staticMeshes_)
	{
		staticMesh->Init();
		engine->AddStaticMeshToRenderer(staticMesh);
	}

	for (SkeletalMesh* skeletalMesh : skeletalMeshes_)
	{
		skeletalMesh->Init();
		engine->AddSkeletalMeshToRenderer(skeletalMesh);
	}

	for (DynamicMesh* dynamicMesh : dynamicMeshes_)
	{
		dynamicMesh->Init();
		engine->AddDynamicMeshToRenderer(dynamicMesh);
	}
}

void Scene::SetShaderDynamicLightUniforms(const Shader* shader) const
{
	for (const Light* light : dynamicDirectionalLights_)
	{
		light->SetShaderUniforms(shader);
	}

	for (const Light* light : dynamicPointLights_)
	{
		light->SetShaderUniforms(shader);
	}

	for (const Light* light : dynamicSpotLights_)
	{
		light->SetShaderUniforms(shader);
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
	case LightMobility::Movable:
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
	case LightMobility::Movable:
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
	case LightMobility::Movable:
		dynamicSpotLights_.push_back(spotLight);
		break;
	}
}