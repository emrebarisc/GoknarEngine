#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

#include "Core.h"
#include "Color.h"

class DirectionalLight;
class PointLight;
class SpotLight;
class Material;
class Mesh;
class ObjectBase;
class Shader;
class Texture;

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

	void AddMesh(Mesh* mesh)
	{
		meshes_.push_back(mesh);
	}

	Mesh* GetMesh(int index)
	{
		return meshes_[index];
	}

	void AddStaticObject(ObjectBase* object)
	{
		staticObjects_.push_back(object);
	}

	ObjectBase* GetStaticObject(int index)
	{
		return staticObjects_[index];
	}

	void AddPointLight(const PointLight* pointLight);

	const std::vector<const PointLight*>& GetStaticPointLights() const
	{
		return staticPointLights_;
	}

	const std::vector<const PointLight*>& GetDynamicPointLights() const
	{
		return dynamicPointLights_;
	}

	void AddDirectionalLight(const DirectionalLight* directionalLight);

	const std::vector<const DirectionalLight*>& GetStaticDirectionalLights() const
	{
		return staticDirectionalLights_;
	}

	const std::vector<const DirectionalLight*>& GetDynamicDirectionalLights() const
	{
		return dynamicDirectionalLights_;
	}

	void AddSpotLight(const SpotLight* spotLight);

	const std::vector<const SpotLight*>& GetStaticSpotLights() const
	{
		return staticSpotLights_;
	}

	const std::vector<const SpotLight*>& GetDynamicSpotLights() const
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

	void AddShader(Shader* shader)
	{
		shaders_.push_back(shader);
	}

	const std::vector<Shader*>& GetShaders() const
	{
		return shaders_;
	}

	Shader* GetShader(int index) const
	{
		return shaders_[index];
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
	std::vector<Shader*> shaders_;
	std::vector<Texture*> textures_;
    std::vector<Mesh*> meshes_;
	std::vector<ObjectBase*> staticObjects_;

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