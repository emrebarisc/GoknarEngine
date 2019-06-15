#ifndef __MESH_H__
#define __MESH_H__

#include "Goknar/Core.h"
#include "Goknar/Math.h"

#include <vector>

class GOKNAR_API Face
{
public:
	unsigned int vertexIndices[3];
};

class GOKNAR_API Mesh
{
public:
	Mesh() : materialId_(0)
	{

	}

	void SetMaterialId(int materialId)
	{
		materialId_ = materialId;
	}

	void AddVertex(const Vector3& vertex)
	{
		vertices_.push_back(vertex);
	}

	const std::vector<Vector3>& GetVertices() const
	{
		return vertices_;
	}

	void AddFace(const Face& face)
	{
		faces_.push_back(face);
	}

private:
	int materialId_;
	std::vector<Vector3> vertices_;
	std::vector<Face> faces_;
};

#endif