#ifndef __MESH_H__
#define __MESH_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Renderer/Renderer.h"

#include <vector>

class MeshUnit;

class GOKNAR_API Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	void AddMesh(MeshUnit* meshUnit);

	virtual void Init();

	void Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix);

protected:

private:
	std::vector<MeshUnit*> subMeshes_;
};

#endif