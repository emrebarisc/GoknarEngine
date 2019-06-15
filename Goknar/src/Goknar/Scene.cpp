#include "pch.h"

#include "Scene.h"
#include "Helpers/SceneParser.h"
#include "Math.h"

Scene *Scene::mainScene = nullptr;

Scene::Scene()
{
	if(mainScene == nullptr)
	{
		mainScene = this;
	}

	backgroundColor_ = Colori(0, 0, 0);

    //ambientLight = Vector3::ZeroVector;
}

Scene::~Scene()
{
    for(auto object : meshes_)
    {
        delete object;
    }

    //for(auto texture : textures)
    //{
    //    delete texture;
    //}
}

void Scene::Init()
{
    //for(auto mesh : meshes)
    //{
    //    mesh->Init();
    //}
}

void Scene::ReadSceneData(char *filePath)
{
    SceneParser::Parse(this, filePath);
}