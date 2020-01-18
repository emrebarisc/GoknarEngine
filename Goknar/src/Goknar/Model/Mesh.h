#ifndef __MESH_H__
#define __MESH_H__

#include "Goknar/Core.h"
#include "Goknar/Math.h"
#include "Goknar/Matrix.h"

#include <vector>

class Shader;
class Material;

class GOKNAR_API Face
{
public:
	unsigned int vertexIndices[3];
};

class GOKNAR_API VertexData
{
public:
	VertexData() : position(Vector3::ZeroVector), normal(Vector3::ZeroVector), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& p) : position(p), normal(Vector3::ZeroVector), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& pos, const Vector3& n) : position(pos), normal(n), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& pos, const Vector3& n, const Vector2& uvCoord) : position(pos), normal(n), uv(uvCoord) { }

	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

typedef std::vector<VertexData> VertexArray;
typedef std::vector<Face> FaceArray;

class GOKNAR_API Mesh
{
public:
	Mesh();

	~Mesh();

	void Init();

	void SetMaterial(/*const*/ Material* material)
	{
		material_ = material;
	}

	Material* GetMaterial()
	{
		return material_;
	}

	void AddVertex(const Vector3& vertex)
	{
		vertices_->push_back(VertexData(vertex));
		vertexCount_++;
	}

	void SetVertexNormal(int index, const Vector3& n)
	{
		vertices_->at(index).normal = n;
	}

	void SetVertexUV(int index, const Vector2& uv)
	{
		vertices_->at(index).uv = uv;
	}

	const VertexArray* GetVerticesPointer() const
	{
		return vertices_;
	}

	void AddFace(const Face& face)
	{
		faces_->push_back(face);
	}

	const FaceArray* GetFacesPointer() const
	{
		return faces_;
	}

	unsigned int GetVertexCount() const
	{
		return vertexCount_;
	}

	unsigned int GetFaceCount() const
	{
		return faceCount_;
	}

	void SetBaseVertex(unsigned int baseVertex)
	{
		baseVertex_ = baseVertex;
	}

	unsigned int GetBaseVertex() const
	{
		return baseVertex_;
	}

	void SetVertexStartingIndex(unsigned int vertexStartingIndex)
	{
		vertexStartingIndex_ = vertexStartingIndex;
	}

	unsigned int GetVertexStartingIndex() const
	{
		return vertexStartingIndex_;
	}

private:
	VertexArray* vertices_;
	FaceArray* faces_;

	Material* material_;

	unsigned int vertexCount_;
	unsigned int faceCount_;

	unsigned int baseVertex_;
	unsigned int vertexStartingIndex_;
};

#endif