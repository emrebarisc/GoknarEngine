#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include "glad/glad.h"

#include <vector>

class GOKNAR_API Mesh;

class GOKNAR_API Renderer
{
public:
	Renderer();
	~Renderer();

	void SetBufferData();
	void Init();

	void Render();

	void AddObjectToRenderer(Mesh* object);

private:
	std::vector<Mesh*> opaqueObjectsToBeRendered_;
	std::vector<Mesh*> maskedObjectsToBeRendered_;
	std::vector<Mesh*> translucentObjectsToBeRendered_;

	unsigned int totalVertexSize_;
	unsigned int totalFaceSize_;

	GEuint vertexBufferId_;
	GEuint indexBufferId_;
};

#endif
