#ifndef __MESHUNIT_H__
#define __MESHUNIT_H__

#include "Goknar/Core.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Math/GoknarMath.h"

#include <vector>

class Shader;
class Material;

enum class GOKNAR_API MeshType
{
	None = 0,
	Static,
	Stream,
	Dynamic
};

class GOKNAR_API Face
{
public:
	Face()
	{
		vertexIndices[0] = 0;
		vertexIndices[1] = 0;
		vertexIndices[2] = 0;
	}

	Face(int faceIndex1, int faceIndex2, int faceIndex3)
	{
		vertexIndices[0] = faceIndex1;
		vertexIndices[1] = faceIndex2;
		vertexIndices[2] = faceIndex3;
	}

	unsigned int vertexIndices[3];
};

// THIS CLASS IS DIRECTLY SENT TO THE GPU
// BEWARE OF ADDING MORE DATA TO GPU SPACE BEFORE ADDING ANOTHER VARIABLE
class GOKNAR_API VertexData
{
public:
	VertexData() : position(Vector3::ZeroVector), normal(Vector3::ZeroVector), color(Vector4::ZeroVector), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& p) : position(p), normal(Vector3::ZeroVector), color(Vector4::ZeroVector), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& pos, const Vector3& n) : position(pos), normal(n), color(Vector4(1.f)), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& pos, const Vector3& n, const Vector4& c) : position(pos), normal(n), color(c), uv(Vector2::ZeroVector) { }
	VertexData(const Vector3& pos, const Vector3& n, const Vector4& c, const Vector2& uvCoord) : position(pos), normal(n), color(c), uv(uvCoord) { }

	Vector4 color;
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

typedef std::vector<VertexData> VertexArray;
typedef std::vector<Face> FaceArray;

class GOKNAR_API MeshUnit : public Content
{
public:
	MeshUnit();

	virtual ~MeshUnit();

	virtual void Init() override;

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	const std::string& GetName() const
	{
		return name_;
	}

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

	void AddVertexData(const VertexData& vertexData)
	{
		vertices_->push_back(vertexData);
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

	void ClearDataFromMemory();

protected:
	unsigned int baseVertex_;
	unsigned int vertexStartingIndex_;

	MeshType meshType_;

	bool isInitialized_;

private:
	VertexArray* vertices_;
	FaceArray* faces_;

	Material* material_;

	std::string name_;

	unsigned int vertexCount_;
	unsigned int faceCount_;
};

#endif