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
    ~Scene();

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

	const std::vector<PointLight*>& GetStaticPointLights() const
	{
		return staticPointLights_;
	}

	const std::vector<PointLight*>& GetDynamicPointLights() const
	{
		return dynamicPointLights_;
	}

	void AddDirectionalLight(DirectionalLight* directionalLight);

	const std::vector<DirectionalLight*>& GetStaticDirectionalLights() const
	{
		return staticDirectionalLights_;
	}

	const std::vector<DirectionalLight*>& GetDynamicDirectionalLights() const
	{
		return dynamicDirectionalLights_;
	}

	void AddSpotLight(SpotLight* spotLight);

	const std::vector<SpotLight*>& GetStaticSpotLights() const
	{
		return staticSpotLights_;
	}

	const std::vector<SpotLight*>& GetDynamicSpotLights() const
	{
		return dynamicSpotLights_;
	}

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
	std::vector<Material*> materials_;
	std::vector<Texture*> textures_;
	std::vector<StaticMesh*> staticMeshes_;
	std::vector<SkeletalMesh*> skeletalMeshes_;
    std::vector<DynamicMesh*> dynamicMeshes_;
	std::vector<ObjectBase*> objects_;

	std::vector<PointLight*> staticPointLights_;
	std::vector<DirectionalLight*> staticDirectionalLights_;
	std::vector<SpotLight*> staticSpotLights_;

	std::vector<PointLight*> dynamicPointLights_;
	std::vector<DirectionalLight*> dynamicDirectionalLights_;
	std::vector<SpotLight*> dynamicSpotLights_;

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif