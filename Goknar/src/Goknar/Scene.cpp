#include "pch.h"

#include "Scene.h"

#include "Engine.h"
#include "Material.h"
#include "Model/Mesh.h"
#include "ObjectBase.h"
#include "Helpers/SceneParser.h"
#include "Renderer/Texture.h"

#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"

Scene::Scene()
{
	backgroundColor_ = Colori(0, 0, 0);
    ambientLight_ = Vector3::ZeroVector;
}

Scene::~Scene()
{
	for (auto object : meshes_)
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

	for (auto object : staticObjects_)
	{
		delete object;
	}
}

void Scene::Init()
{
	for (Mesh* mesh : meshes_)
	{
		mesh->Init();
		engine->AddObjectToRenderer(mesh);
	}

	for (Texture* texture : textures_)
	{
		texture->Init();
	}

	for (Material* material : materials_)
	{
		material->Init();
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

void Scene::ReadSceneData(char *filePath)
{
    SceneParser::Parse(this, filePath);
}

void Scene::AddPointLight(const PointLight* pointLight)
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

void Scene::AddDirectionalLight(const DirectionalLight* directionalLight)
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


void Scene::AddSpotLight(const SpotLight* spotLight)
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