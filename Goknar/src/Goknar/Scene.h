/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

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
//#include "MeshBase.h"

class BRDF;
class MeshBase;
class Texture;

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

 //   std::vector<Camera> cameras;
 //   std::vector<Light *> lights;
 //   std::vector<Material> materials;
 //   std::vector<BRDF *> BRDFs;
 //   std::vector<VertexData> vertices;
 //   std::vector<Texture *> textures;
 //   std::vector<Vector3> translations;
 //   std::vector<Vector3> scalings;
 //   std::vector<Vector4> rotations;
 //   std::vector<MeshBase *> meshes;

 //   Vector3 ambientLight;

    Colori bgColor;

	//Camera *mainCamera = nullptr;
};

#endif