/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#include "Scene.h"
//
//#include "MeshBase.h"
//#include "SceneParser.h"
#include "Math.h"

Scene *Scene::mainScene = nullptr;

Scene::Scene()
{
	//if(mainScene == nullptr)
	//{
	//	mainScene = this;
	//}

 //   ambientLight = Vector3::ZeroVector;
}

Scene::~Scene()
{
    //for(auto object : meshes)
    //{
    //    delete object;
    //}

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
    //SceneParser::Parse(this, filePath);
}