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

class Mesh;

/*
    Scene class containing all the scene data
*/
class GOKNAR_API Scene
{
public:
	static Scene *mainScene;

    Scene();
    ~Scene();

    void Init();

    // Scene data parser
    void ReadSceneData(char *filePath);

	void AddMesh(const Mesh* mesh)
	{
		meshes_.push_back(mesh);
	}

	void SetAmbientLight(const Vector3& ambientLight)
	{
		ambientLight_ = ambientLight;
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

 //   std::vector<Camera> cameras;
 //   std::vector<Light *> lights;
 //   std::vector<Material> materials;
 //   std::vector<Texture *> textures;
 //   std::vector<Vector3> translations;
 //   std::vector<Vector3> scalings;
 //   std::vector<Vector4> rotations;
    std::vector<const Mesh *> meshes_;

	Vector3 ambientLight_;

    Colori backgroundColor_;
};

#endif