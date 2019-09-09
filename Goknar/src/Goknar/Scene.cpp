#include "pch.h"

#include "Scene.h"

#include "Helpers/SceneParser.h"

#include "Engine.h"
#include "Math.h"

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
}

void Scene::Init()
{
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