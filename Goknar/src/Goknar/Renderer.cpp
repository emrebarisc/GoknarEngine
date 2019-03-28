/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/
#include "pch.h"

#include "Renderer.h"
#include "Engine.h"
#include "Scene.h"
#include "glad/glad.h"

Renderer::Renderer(): vertexBufferId_(0), indexBufferId_(0)
{
	graphicsManager_ = engine->GetGraphicsManager();
}

Renderer::~Renderer() = default;

void Renderer::SetBufferData()
{
}

void Renderer::Init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
}

void Renderer::Render()
{
	glClearColor(Scene::mainScene->bgColor.r, Scene::mainScene->bgColor.g, Scene::mainScene->bgColor.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO Render scene
}

void Renderer::AddObjectToRenderer(RenderingObject *object)
{
	//objectsToBeRendered_.push_back(object);
}
