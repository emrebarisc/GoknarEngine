#ifndef __SCENE_H__
#define __SCENE_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "Core.h"
#include "Color.h"
#include "Goknar/Math/Quaternion.h"

class DirectionalLight;
class PointLight;
class ReflectionProbe;
class SpotLight;
class Material;
class NavigationMesh;
class ObjectBase;
class Texture;

class DynamicMesh;
class StaticMesh;
class SkeletalMesh;

struct NavMeshSettings;

struct GOKNAR_API SceneReference
{
	std::string path;
	Vector3 relativePosition{ Vector3::ZeroVector };
	Quaternion relativeRotation{ Quaternion::Identity };
	Vector3 relativeScaling{ Vector3(1.f) };
	ObjectBase* sceneRootObject{ nullptr };
};

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
	void RebuildNavigationMesh(const NavMeshSettings& settings, bool removeNavigationTreeObjects = false);

	void AddObject(ObjectBase* object, bool isFromReferencedScene = false);
	void RemoveObject(ObjectBase* object);
	void DestroyObjects();

	const std::vector<ObjectBase*>& GetObjects() const
	{
		return objects_;
	}

	NavigationMesh* GetNavigationMesh() const
	{
		return navigationMesh_;
	}

	bool GetIsObjectFromReferencedScene(ObjectBase* object) const;

	void AddSceneReference(const SceneReference& sceneReference);

	const std::vector<SceneReference>& GetSceneReferences() const
	{
		return sceneReferences_;
	}

	void SetPath(const std::string& path)
	{
		path_ = path;
	}

	const std::string& GetPath() const
	{
		return path_;
	}

	void AddPointLight(PointLight* pointLight, bool isFromReferencedScene = false);
	void RemovePointLight(PointLight* pointLight);
	bool GetIsPointLightFromReferencedScene(PointLight* pointLight) const;

	const std::vector<PointLight*>& GetPointLights() const
	{
		return pointLights_;
	}

	void AddDirectionalLight(DirectionalLight* directionalLight, bool isFromReferencedScene = false);
	void RemoveDirectionalLight(DirectionalLight* directionalLight);
	bool GetIsDirectionalLightFromReferencedScene(DirectionalLight* directionalLight) const;

	const std::vector<DirectionalLight*>& GetDirectionalLights() const
	{
		return directionalLights_;
	}

	void AddSpotLight(SpotLight* spotLight, bool isFromReferencedScene = false);
	void RemoveSpotLight(SpotLight* spotLight);
	bool GetIsSpotLightFromReferencedScene(SpotLight* spotLight) const;

	const std::vector<SpotLight*>& GetSpotLights() const
	{
		return spotLights_;
	}

	void AddReflectionProbe(ReflectionProbe* reflectionProbe);
	void RemoveReflectionProbe(ReflectionProbe* reflectionProbe);

	const std::vector<ReflectionProbe*>& GetReflectionProbes() const
	{
		return reflectionProbes_;
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
	std::string path_;

	std::vector<ObjectBase*> objects_;
	std::unordered_map<ObjectBase*, bool> objectReferencedSceneState_;
	std::vector<SceneReference> sceneReferences_;

	std::vector<Texture*> textures_;

	std::vector<PointLight*> pointLights_;
	std::unordered_map<PointLight*, bool> pointLightReferencedSceneState_;
	std::vector<DirectionalLight*> directionalLights_;
	std::unordered_map<DirectionalLight*, bool> directionalLightReferencedSceneState_;
	std::vector<SpotLight*> spotLights_;
	std::unordered_map<SpotLight*, bool> spotLightReferencedSceneState_;
	std::vector<ReflectionProbe*> reflectionProbes_;
	NavigationMesh* navigationMesh_{ nullptr };

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif
