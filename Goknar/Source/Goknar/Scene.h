#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

#include "Core.h"
#include "Color.h"

class DirectionalLight;
class PointLight;
class SpotLight;
class Material;
class ObjectBase;
class Texture;

class DynamicMesh;
class StaticMesh;
class SkeletalMesh;

/*
    Scene class containing all the scene data
*/
class GOKNAR_API Scene
{
public:
    Scene();
    virtual ~Scene();

    void PreInit();
    void Init();
    void PostInit();

    // Scene data parser
    void ReadSceneData(const std::string& filePath);

	void AddStaticMesh(StaticMesh* staticMesh)
	{
		staticMeshes_.push_back(staticMesh);
	}

	// TODO: TEST
	void DeleteStaticMesh(StaticMesh* staticMesh);

	StaticMesh* GetStaticMesh(int index)
	{
		return staticMeshes_[index];
	}

	void AddSkeletalMesh(SkeletalMesh* skeletalMesh)
	{
		skeletalMeshes_.push_back(skeletalMesh);
	}

	// TODO: TEST
	void DeleteSkeletalMesh(SkeletalMesh* skeletalMesh);

	SkeletalMesh* GetSkeletalMesh(int index)
	{
		return skeletalMeshes_[index];
	}

	void AddDynamicMesh(DynamicMesh* dynamicMesh)
	{
		dynamicMeshes_.push_back(dynamicMesh);
	}

	// TODO: TEST
	void DeleteDynamicMesh(DynamicMesh* dynamicMesh);

	DynamicMesh* GetDynamicMesh(int index)
	{
		return dynamicMeshes_[index];
	}

	void AddObject(ObjectBase* object)
	{
		objects_.push_back(object);
	}

	bool RemoveObject(ObjectBase* object)
	{
		decltype(objects_.cbegin()) objectsIterator = objects_.cbegin();
		while (objectsIterator != objects_.end())
		{
			if (*objectsIterator == object)
			{
				objects_.erase(objectsIterator);
				return true;
			}
			std::advance(objectsIterator, 1);
		}

		return false;
	}

	void ClearObjects()
	{
		objects_.clear();
	}

	ObjectBase* GetStaticObject(int index)
	{
		return objects_[index];
	}

	void AddPointLight(PointLight* pointLight);

	const std::vector<PointLight*>& GetPointLights() const
	{
		return pointLights_;
	}

	void AddDirectionalLight(DirectionalLight* directionalLight);

	const std::vector<DirectionalLight*>& GetDirectionalLights() const
	{
		return directionalLights_;
	}

	void AddSpotLight(SpotLight* spotLight);

	const std::vector<SpotLight*>& GetSpotLights() const
	{
		return spotLights_;
	}

	void AddTexture(Texture* texture)
	{
		textures_.push_back(texture);
	}

	const std::vector<Texture*>& GetTextures() const
	{
		return textures_;
	}

	const Texture* GetTexture(int index) const
	{
		return textures_[index];
	}

	void SetAmbientLight(const Vector3& ambientLight)
	{
		ambientLight_ = ambientLight;
	}

	const Vector3& GetAmbientLight() const
	{
		return ambientLight_;
	}

	void SetBackgroundColor(const Colorf& color)
	{
		backgroundColor_ = color;
	}

	const Colorf& GetBackgroundColor() const
	{
		return backgroundColor_;
	}

private:
	std::vector<Texture*> textures_;
	std::vector<StaticMesh*> staticMeshes_;
	std::vector<SkeletalMesh*> skeletalMeshes_;
    std::vector<DynamicMesh*> dynamicMeshes_;
	std::vector<ObjectBase*> objects_;

	std::vector<PointLight*> pointLights_;
	std::vector<DirectionalLight*> directionalLights_;
	std::vector<SpotLight*> spotLights_;

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif