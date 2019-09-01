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
class PointLight;

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

	const std::vector<const PointLight *>& GetPointLights() const
	{
		return pointLights_;
	}

	void SetAmbientLight(const Vector3& ambientLight)
	{
		ambientLight_ = ambientLight;
	}

	const Vector3& GetAmbientLight() const
	{
		return ambientLight_;
	}

	void SetBackgroundColor(const Colori& color)
	{
		backgroundColor_ = color;
	}

	const Colori& GetBackgroundColor() const
	{
		return backgroundColor_;
	}

private:
	std::vector<const Material*> materials_;
    std::vector<const Mesh*> meshes_;
	std::vector<const PointLight*> pointLights_;

	Vector3 ambientLight_;

    Colori backgroundColor_;
};

#endif