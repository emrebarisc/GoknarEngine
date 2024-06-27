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

	void AddPointLight(PointLight* pointLight);
	void RemovePointLight(PointLight* pointLight);

	const std::vector<PointLight*>& GetPointLights() const
	{
		return pointLights_;
	}

	void AddDirectionalLight(DirectionalLight* directionalLight);
	void RemoveDirectionalLight(DirectionalLight* directionalLight);

	const std::vector<DirectionalLight*>& GetDirectionalLights() const
	{
		return directionalLights_;
	}

	void AddSpotLight(SpotLight* spotLight);
	void RemoveSpotLight(SpotLight* spotLight);

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

	std::vector<PointLight*> pointLights_;
	std::vector<DirectionalLight*> directionalLights_;
	std::vector<SpotLight*> spotLights_;

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif