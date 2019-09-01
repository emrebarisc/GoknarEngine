#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

//#include "Camera.h"
#include "Core.h"
#include "Color.h"
//#include "Material.h"
//#include "Math.h"
//#include "Light.h"
//#include "Texture.h"

class Material;
class Mesh;

class DirectionalLight;
class PointLight;
class SpotLight;

/*
    Scene class containing all the scene data
*/
class GOKNAR_API Scene
{
public:
    Scene();
    ~Scene();

    void Init();

    // Scene data parser
    void ReadSceneData(char *filePath);

	void AddMesh(const Mesh* mesh)
	{
		meshes_.push_back(mesh);
	}

	void AddPointLight(const PointLight* pointLight)
	{
		pointLights_.push_back(pointLight);
	}

	const std::vector<const PointLight*>& GetPointLights() const
	{
		return pointLights_;
	}

	void AddDirectionalLight(const DirectionalLight* pointLight)
	{
		directionalLights_.push_back(pointLight);
	}

	const std::vector<const DirectionalLight*>& GetDirectionalLights() const
	{
		return directionalLights_;
	}

	void AddSpotLight(const SpotLight* pointLight)
	{
		spotLights_.push_back(pointLight);
	}

	const std::vector<const SpotLight*>& GetSpotLights() const
	{
		return spotLights_;
	}

	void AddMaterial(const Material* material)
	{
		materials_.push_back(material);
	}

	const std::vector<const Material*>& GetMaterials() const
	{
		return materials_;
	}

	const Material* GetMaterial(int index) const
	{
		return materials_[index];
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
	std::vector<const Material*> materials_;
    std::vector<const Mesh*> meshes_;
	std::vector<const PointLight*> pointLights_;
	std::vector<const DirectionalLight*> directionalLights_;
	std::vector<const SpotLight*> spotLights_;

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif