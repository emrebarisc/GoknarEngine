#include "pch.h"

#include "Application.h"

#include <filesystem>

#include "Engine.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Helpers/SceneParser.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Log.h"
#include "Scene.h"

Application::Application() :
	applicationType_(AppType::Application3D)
{
	mainScene_ = new Scene();
}

Application::~Application()
{
	delete mainScene_;
}

void Application::PreInit()
{
	mainScene_->PreInit();
	isPreInitialized_ = true;
}

void Application::Init()
{
	mainScene_->Init();
	isInitialized_ = true;
}

void Application::PostInit()
{
	mainScene_->PostInit();
	isPostInitialized_ = true;
}

void Application::Run()
{

}

bool Application::OpenScene(const std::string& path)
{
	const std::string relativeScenePath = ContentPathUtils::ToContentRelativePath(path);
	if (relativeScenePath.empty())
	{
		GOKNAR_CORE_WARN("Application::OpenScene called with an empty path.");
		return false;
	}

	const std::string normalizedRelativeScenePath = ContentPathUtils::NormalizePath(relativeScenePath);
	if (mainScene_ && ContentPathUtils::NormalizePath(mainScene_->GetPath()) == normalizedRelativeScenePath)
	{
		return true;
	}

	const std::string absoluteScenePath = ContentPathUtils::ToAbsoluteContentPath(normalizedRelativeScenePath);
	if (!std::filesystem::exists(absoluteScenePath))
	{
		GOKNAR_CORE_WARN("Scene could not be found at %s.", absoluteScenePath.c_str());
		return false;
	}

	Scene* previousScene = mainScene_;
	if (previousScene)
	{
		previousScene->DestroyObjects();
		engine->FlushPendingDestroy();
		delete previousScene;
	}

	SceneParser::ClearCaches();

	mainScene_ = new Scene();
	mainScene_->ReadSceneData(normalizedRelativeScenePath);

	if (isPreInitialized_)
	{
		mainScene_->PreInit();
	}

	if (isInitialized_)
	{
		mainScene_->Init();
	}

	if (isPostInitialized_)
	{
		mainScene_->PostInit();
		engine->GetResourceManager()->InitializePendingMaterials();
		engine->InitializePendingObjectsAndComponents();
	}

	return true;
}
