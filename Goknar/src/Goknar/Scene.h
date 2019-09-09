#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

#include "Core.h"
#include "Color.h"

#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"

class Material;
class Mesh;

/*
    Scene class containing all the scene data
*/
class GOKNAR_API Scene
{
public:
    Scene();
    ~Scene();

    void Init();

	void SetShaderDynamicLightUniforms(const Shader* shader) const;

    // Scene data parser
    void ReadSceneData(char *filePath);

	void AddMesh(const Mesh* mesh)
	{
		meshes_.push_back(mesh);
	}

	void AddPointLight(const PointLight* pointLight)
	{
		switch (pointLight->GetLightMobility())
		{
		case LightMobility::Static:
			staticPointLights_.push_back(pointLight);
			break;
		case LightMobility::Movable:
			dynamicPointLights_.push_back(pointLight);
			break;
		}

	}

	const std::vector<const PointLight*>& GetStaticPointLights() const
	{
		return staticPointLights_;
	}

	const std::vector<const PointLight*>& GetDynamicPointLights() const
	{
		return dynamicPointLights_;
	}

	void AddDirectionalLight(const DirectionalLight* directionalLight)
	{
		switch (directionalLight->GetLightMobility())
		{
		case LightMobility::Static:
			staticDirectionalLights_.push_back(directionalLight);
			break;
		case LightMobility::Movable:
			dynamicDirectionalLights_.push_back(directionalLight);
			break;
		}
	}

	const std::vector<const DirectionalLight*>& GetStaticDirectionalLights() const
	{
		return staticDirectionalLights_;
	}

	const std::vector<const DirectionalLight*>& GetDynamicDirectionalLights() const
	{
		return dynamicDirectionalLights_;
	}

	void AddSpotLight(const SpotLight* spotLight)
	{
		switch (spotLight->GetLightMobility())
		{
		case LightMobility::Static:
			staticSpotLights_.push_back(spotLight);
			break;
		case LightMobility::Movable:
			dynamicSpotLights_.push_back(spotLight);
			break;
		}
	}

	const std::vector<const SpotLight*>& GetStaticSpotLights() const
	{
		return staticSpotLights_;
	}

	const std::vector<const SpotLight*>& GetDynamicSpotLights() const
	{
		return dynamicSpotLights_;
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

	std::vector<const PointLight*> staticPointLights_;
	std::vector<const DirectionalLight*> staticDirectionalLights_;
	std::vector<const SpotLight*> staticSpotLights_;

	std::vector<const PointLight*> dynamicPointLights_;
	std::vector<const DirectionalLight*> dynamicDirectionalLights_;
	std::vector<const SpotLight*> dynamicSpotLights_;

	Vector3 ambientLight_;

    Colorf backgroundColor_;
};

#endif