#include "pch.h"

#include "Managers/ResourceManager.h"

#include "Log.h"

#include "Contents/Audio.h"
#include "Contents/Image.h"
#include "Model/StaticMesh.h"
#include "IO/IOManager.h"
#include "Materials/Material.h"

ResourceManager::ResourceManager() :
	resourceContainer_(new ResourceContainer())
{
}

ResourceManager::~ResourceManager()
{
	delete resourceContainer_;

	for (auto material : materials_)
	{
		delete material;
	}
}

void ResourceManager::PreInit()
{
	resourceContainer_->PreInit();
}

void ResourceManager::Init()
{
	resourceContainer_->Init();
}

void ResourceManager::PostInit()
{
	resourceContainer_->PostInit();
}

Content* ResourceManager::LoadContent(const std::string& path)
{
	Content* content = nullptr;

	ResourceType type = ResourceManagerUtils::GetResourceType(path);
	switch (type)
	{
	case ResourceType::Image:
	{
		Image* image = IOManager::LoadImage(path);
		if (image)
		{
			image->SetPath(path);
			resourceContainer_->AddImage(image);
			content = image;
		}
		break;
	}
	case ResourceType::Model:
	{
		//ModelPrimitiveData* modelPrimitiveData = LoadModelPrimitiveData(path);
		//if (modelPrimitiveData.HasAnimation())
		//{
		//	MeshUnit* mesh = modelPrimitiveData->ToMesh();
		//	resourceContainer_->AddMesh(mesh);
		//	content = mesh;
		//}
		//else
		//{
		//	AnimationData* animationData = modelPrimitiveData->ToAnimationData();
		//	resourceContainer_->AddAnimationData(animationData);
		//	content = animationData;
		//}
		MeshUnit* mesh = IOManager::LoadModel(path);
		if (mesh)
		{
			mesh->SetPath(path);
			resourceContainer_->AddMesh(mesh);
			content = mesh;
		}
		break;
	}
	case ResourceType::Audio:
	{
		//Audio* audio = IOManager::LoadAudio(path);
		//if(audio)
		//{
		//	audio->SetPath(path); 
		//	resourceContainer_->AddAudio(audio);
		//	content = audio;
		//}
		break;
	}
	case ResourceType::None:
	default:
		break;
	}

	if (content)
	{
		resourceContainer_->contentPathMap_[path] = content;
	}

	return content;
}

ResourceContainer::ResourceContainer()
{
}

ResourceContainer::~ResourceContainer()
{
	for (Image* image : imageArray_)
	{
		delete image;
	}

	for (MeshUnit* mesh : meshArray_)
	{
		delete mesh;
	}

	for (Audio* audio : audioArray_)
	{
		delete audio;
	}
}

void ResourceContainer::PreInit()
{
	for (Image* image : imageArray_)
	{
		image->PreInit();
	}

	for (MeshUnit* mesh : meshArray_)
	{
		mesh->PreInit();
	}

	for (Audio* audio : audioArray_)
	{
		audio->PreInit();
	}
}

void ResourceContainer::Init()
{
	for (Image* image : imageArray_)
	{
		image->Init();
	}

	for (MeshUnit* mesh : meshArray_)
	{
		mesh->Init();
	}

	for (Audio* audio : audioArray_)
	{
		audio->Init();
	}
}

void ResourceContainer::PostInit()
{
	for (Image* image : imageArray_)
	{
		image->PostInit();
	}

	for (MeshUnit* mesh : meshArray_)
	{
		mesh->PostInit();
	}

	for (Audio* audio : audioArray_)
	{
		audio->PostInit();
	}
}

void ResourceContainer::AddImage(Image* image)
{
	GOKNAR_CORE_ASSERT(!image->GetPath().empty());

	imageArray_.push_back(image);
	contentPathMap_[image->GetPath()] = image;
}

void ResourceContainer::AddMesh(MeshUnit* mesh)
{
	GOKNAR_CORE_ASSERT(!mesh->GetPath().empty());

	meshArray_.push_back(mesh);
	contentPathMap_[mesh->GetPath()] = mesh;
}

void ResourceContainer::AddAudio(Audio* audio)
{
	GOKNAR_CORE_ASSERT(!audio->GetPath().empty());

	audioArray_.push_back(audio);
	contentPathMap_[audio->GetPath()] = audio;
}

ResourceType ResourceManagerUtils::GetResourceType(const std::string& path)
{
	std::string extension = GetExtension(path);

	std::transform(
		extension.begin(),
		extension.end(),
		extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (extension == "jpg" || extension == "png")
	{
		return ResourceType::Image;
	}
	else if (extension == "fbx")
	{
		return ResourceType::Model;
	}
	else if (extension == "wav")
	{
		return ResourceType::Audio;
	}
	return ResourceType::None;
}

std::string ResourceManagerUtils::GetExtension(const std::string& path)
{
	int indexAfterTheLastDot = path.find_last_of('.') + 1;
	return path.substr(indexAfterTheLastDot, path.size() - indexAfterTheLastDot);
}