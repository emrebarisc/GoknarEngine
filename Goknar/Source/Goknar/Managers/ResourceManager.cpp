#include "pch.h"

#include "Managers/ResourceManager.h"

#include "GoknarAssert.h"
#include "Contents/Audio.h"
#include "Contents/Content.h"
#include "Contents/Image.h"
#include "Renderer/TextureAtlasManager.h"
#include "IO/IOManager.h"

ResourceManager::ResourceManager() :
	resourceContainer_(new ResourceContainer())
{
}

ResourceManager::~ResourceManager()
{
	materials_.clear();
	delete resourceContainer_;
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
	InitializePendingMaterials();
}

void ResourceManager::InitializePendingMaterials()
{
	size_t materialIndex = 0;
	while (materialIndex < materials_.size())
	{
		Material* material = materials_[materialIndex].get();

		if (!material->GetIsInitialized())
		{
			material->Build(nullptr);
			material->PreInit();
			material->Init();
			material->PostInit();
		}

		++materialIndex;
	}
}

void ResourceManager::RemoveMaterial(Material* material)
{
	auto materialIterator =
		std::find_if(
			materials_.begin(),
			materials_.end(),
			[material](const std::unique_ptr<Material>& candidate)
			{
				return candidate.get() == material;
			});

	if (materialIterator != materials_.end())
	{
		materials_.erase(materialIterator);
	}
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
		Content* mesh = IOManager::LoadModel(path);
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

ResourceContainer::ResourceContainer() :
	imageTextureAtlasManager_(new TextureAtlasManager())
{
}

ResourceContainer::~ResourceContainer()
{
	delete imageTextureAtlasManager_;

	for (Image* image : imageArray_)
	{
		delete image;
	}

	for (Content* mesh : meshArray_)
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
	isPreInitialized_ = true;

	for (Image* image : imageArray_)
	{
		if (image->HasTextureAtlasCategories())
		{
			for (TextureAtlasCategory category : image->GetTextureAtlasCategories())
			{
				RegisterImageToTextureAtlas(image, category);
			}
		}
		else
		{
			RegisterImageToTextureAtlas(image);
		}
	}

	FlushImageTextureAtlas();

	for (Image* image : imageArray_)
	{
		image->PreInit();
	}

	for (Content* mesh : meshArray_)
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
	isInitialized_ = true;

	if (imageTextureAtlasManager_)
	{
		imageTextureAtlasManager_->Init();
	}

	for (Image* image : imageArray_)
	{
		image->Init();
	}

	for (Content* mesh : meshArray_)
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
	isPostInitialized_ = true;

	if (imageTextureAtlasManager_)
	{
		imageTextureAtlasManager_->PostInit();
	}

	for (Image* image : imageArray_)
	{
		image->PostInit();
	}

	for (Content* mesh : meshArray_)
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
	RegisterImageToTextureAtlas(image);
}

bool ResourceContainer::RegisterImageToTextureAtlas(Image* image, TextureAtlasCategory category)
{
	if (!imageTextureAtlasManager_ || !image)
	{
		return false;
	}

	if (!(image->GetCanUseTextureAtlas() || useTextureAtlasForAllImages_))
	{
		return false;
	}

	image->AddTextureAtlasCategory(category);
	return imageTextureAtlasManager_->AddImage(image, category);
}

void ResourceContainer::FlushImageTextureAtlas()
{
	// AssetParser can register atlas images while asset XML is being parsed,
	// before the renderer has a valid OpenGL context. In that phase we only
	// collect registrations. The first real GPU upload happens from
	// ResourceContainer::PreInit(), where the engine's GL entry points are ready.
	if (!isPreInitialized_ || !imageTextureAtlasManager_ || !imageTextureAtlasManager_->HasImages())
	{
		return;
	}

	imageTextureAtlasManager_->PreInit();

	if (isInitialized_)
	{
		imageTextureAtlasManager_->Init();
	}

	if (isPostInitialized_)
	{
		imageTextureAtlasManager_->PostInit();
	}
}

TextureAtlas* ResourceContainer::GetImageTextureAtlas(TextureAtlasCategory category) const
{
	return imageTextureAtlasManager_ ? imageTextureAtlasManager_->GetFirstAtlas(category) : nullptr;
}

void ResourceContainer::AddMesh(Content* mesh)
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
