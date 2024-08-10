#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include "Core.h"

#include <map>

class Audio;
class Content;
class Material;
class MeshUnit;
class Image;

enum class GOKNAR_API ResourceType : unsigned char
{
	None = 0,
	Image,
	Model,
	Audio
};

class GOKNAR_API ResourceManagerUtils
{
public:
	static ResourceType GetResourceType(const std::string& path);
	static std::string GetExtension(const std::string& path);
protected:

private:
};

class GOKNAR_API ResourceContainer
{
	friend class ResourceManager;

public:
	ResourceContainer();
	~ResourceContainer();

	void PreInit();
	void Init();
	void PostInit();

	void AddImage(Image* image);
	Image* GetImage(int index) const
	{
		if (imageArray_.size() <= index)
		{
			return nullptr;
		}
		return imageArray_[index];
	}

	void AddMesh(MeshUnit* mesh);
	MeshUnit* GetMesh(int index)
	{
		if (meshArray_.size() <= index)
		{
			return nullptr;
		}
		return meshArray_[index];
	}

	void AddAudio(Audio* audio);
	Audio* GetAudio(int index)
	{
		if (audioArray_.size() <= index)
		{
			return nullptr;
		}
		return audioArray_[index];
	}

	template<class T>
	inline T* GetContent(const std::string& path)
	{
		if (contentPathMap_.find(path) == contentPathMap_.end())
		{
			return nullptr;
		}

		return dynamic_cast<T*>(contentPathMap_[path]);
	}

	const std::vector<Image*>& GetImageArray() const
	{
		return imageArray_;
	}

	const std::vector<MeshUnit*>& GetMeshArray() const
	{
		return meshArray_;
	}

	const std::vector<Audio*>& GetAudioArray() const
	{
		return audioArray_;
	}

protected:

private:
	std::map<std::string, Content*> contentPathMap_;

	std::vector<Image*> imageArray_;
	std::vector<MeshUnit*> meshArray_;
	std::vector<Audio*> audioArray_;
};

class GOKNAR_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void PreInit();
	void Init();
	void PostInit();

	ResourceContainer* GetResourceContainer() const
	{
		return resourceContainer_;
	}

	template <class T>
	T* GetContent(const std::string& path)
	{
		std::string fullPath = ContentDir + path;

		T* content = resourceContainer_->GetContent<T>(fullPath);
		if (content)
		{
			return content;
		}

		return dynamic_cast<T*>(LoadContent(fullPath));
	}

#if defined(GOKNAR_BUILD_DEBUG)
	template <class T>
	T* GetEngineContent(const std::string& path)
	{
		std::string fullPath = EngineContentDir + path;

		T* content = resourceContainer_->GetContent<T>(fullPath);
		if (content)
		{
			return content;
		}

		return dynamic_cast<T*>(LoadContent(fullPath));
	}
#endif

	void AddMaterial(Material* material)
	{
		materials_.push_back(material);
	}

	const std::vector<Material*>& GetMaterials() const
	{
		return materials_;
	}

	Material* GetMaterial(int index) const
	{
		return materials_[index];
	}

private:
	Content* LoadContent(const std::string& path);

	std::vector<Material*> materials_;

	ResourceContainer* resourceContainer_;
};

#endif
