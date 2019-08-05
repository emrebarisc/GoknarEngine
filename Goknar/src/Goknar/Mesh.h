#ifndef __MESH_H__
#define __MESH_H__

#include "Goknar/Core.h"
#include "Goknar/Math.h"
#include "Goknar/Matrix.h"

#include "glad/glad.h"

#include <vector>

class GOKNAR_API Face
{
public:
	unsigned int vertexIndices[3];
};

class GOKNAR_API Mesh
{
public:
	Mesh();

	~Mesh();

	void Init();

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

	GLuint GetVertexArrayId() const
	{
		return vertexArrayId_;
	}

	GLuint GetVertexBufferId() const
	{
		return vertexBufferId_;
	}

	GLuint GetIndexBufferId() const
	{
		return indexBufferId_;
	}

	void Render() const;

private:
	Matrix modelMatrix_;

	std::vector<Vector3> vertices_;
	std::vector<Face> faces_;

	class Shader* shader_;

	GLuint vertexArrayId_;
	GLuint vertexBufferId_;
	GLuint indexBufferId_;

	int materialId_;
};

#endif