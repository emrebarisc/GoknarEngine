#ifndef __MESH_H__
#define __MESH_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

#include <vector>

class MeshUnit;

class GOKNAR_API Mesh
{
public:
	Mesh();
	~Mesh();

	void AddMesh(MeshUnit* meshUnit);

	virtual void Init();

	void Render(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix);

protected:

private:
	std::vector<MeshUnit*> subMeshes_;
};

#endif