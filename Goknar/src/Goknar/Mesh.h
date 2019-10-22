#ifndef __MESH_H__
#define __MESH_H__

#include "Component.h"

#include "Goknar/Core.h"
#include "Goknar/Math.h"
#include "Goknar/Matrix.h"

#include "glad/glad.h"

#include <vector>

class Shader;

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

class GOKNAR_API Mesh : public Component
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
		vertices_->push_back(VertexData(vertex));
	}

	void SetVertexNormal(int index, const Vector3& n)
	{
		vertices_->at(index).normal = n;
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

	const Shader* GetShader() const
	{
		return shader_;
	}

	int GetVertexCount() const
	{
		return vertexCount_;
	}

	int GetFaceCount() const
	{
		return faceCount_;
	}

	void Render() const;

private:
	Matrix modelMatrix_;

	VertexArray* vertices_;
	FaceArray* faces_;

	Shader* shader_;

	unsigned int vertexCount_;
	unsigned int faceCount_;

	int materialId_;
};

#endif