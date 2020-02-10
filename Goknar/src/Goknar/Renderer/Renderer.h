#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "glad/glad.h"

#include <vector>

class GOKNAR_API Mesh;
class GOKNAR_API MeshInstance;

class GOKNAR_API Renderer
{
public:
	Renderer();
	~Renderer();

	void SetBufferData();
	void Init();

	void Render();

	void AddMeshToRenderer(Mesh* object);
	void AddMeshInstance(MeshInstance* object);
	void RemoveMeshInstance(MeshInstance* object);

private:
	std::vector<Mesh*> meshes_;

	std::vector<MeshInstance*> opaqueMeshInstances_;
	//TODO: Is it really necessary to hold masked objects as a seperate array?
	std::vector<MeshInstance*> maskedMeshInstances_;
	std::vector<MeshInstance*> translucentMeshInstances_;

	unsigned int totalVertexSize_;
	unsigned int totalFaceSize_;

	GEuint vertexBufferId_;
	GEuint indexBufferId_;
};

#endif
