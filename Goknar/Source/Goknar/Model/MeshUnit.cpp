#include "pch.h"

#include "MeshUnit.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Materials/MaterialBase.h"
#include "Scene.h"
#include "Materials/Material.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"

namespace
{
	Vector3 GetSafeNormal(const Vector3& normal)
	{
		if (SMALLER_EPSILON < normal.SquareLength())
		{
			return normal.GetNormalized();
		}

		return Vector3::ForwardVector;
	}

	Vector3 GetFallbackTangent(const Vector3& normal)
	{
		return normal.GetOrthonormalBasis();
	}

	Vector3 OrthogonalizeTangent(const Vector3& normal, const Vector3& tangent)
	{
		Vector3 orthogonalTangent = tangent - normal * normal.Dot(tangent);
		if (SMALLER_EPSILON < orthogonalTangent.SquareLength())
		{
			return orthogonalTangent.GetNormalized();
		}

		return GetFallbackTangent(normal);
	}
}

MeshUnit::MeshUnit() :
	material_(nullptr), 
	vertexCount_(0), 
	faceCount_(0),
	baseVertex_(0),
	vertexStartingIndex_(0),
	isInitialized_(false),
	meshType_(MeshType::None)
{
	vertices_ = new VertexArray();
	faces_ = new FaceArray();
}

MeshUnit::~MeshUnit()
{
	if (vertices_)
	{
		delete vertices_;
	}

	if (faces_)
	{
		delete faces_;
	}
}

void MeshUnit::PreInit()
{
	GenerateTangents();

	aabb_.CalculateSize();

	vertexCount_ = (int)vertices_->size();
	faceCount_ = (int)faces_->size();

	if (material_)
	{
		material_->Build(this);
		material_->PreInit();
	}
}

void MeshUnit::Init()
{
	if (material_)
	{
		material_->Init();
	}
}

void MeshUnit::PostInit()
{
	if (material_)
	{
		material_->PostInit();
	}

	isInitialized_ = true;
}

void MeshUnit::ClearDataFromMemory()
{
	vertices_->clear();
	delete vertices_;
	vertices_ = nullptr;

	faces_->clear();
	delete faces_;
	faces_ = nullptr;
}

const IMaterialBase* MeshUnit::GetMaterialBase() const
{
	return material_;
}

void MeshUnit::GenerateTangents()
{
	if (!vertices_ || vertices_->empty())
	{
		return;
	}

	std::vector<Vector3> tangentAccumulators(vertices_->size(), Vector3::ZeroVector);
	std::vector<Vector3> bitangentAccumulators(vertices_->size(), Vector3::ZeroVector);

	if (faces_)
	{
		for (const Face& face : *faces_)
		{
			const unsigned int index0 = face.vertexIndices[0];
			const unsigned int index1 = face.vertexIndices[1];
			const unsigned int index2 = face.vertexIndices[2];
			if (index0 >= vertices_->size() || index1 >= vertices_->size() || index2 >= vertices_->size())
			{
				continue;
			}

			const VertexData& vertex0 = vertices_->at(index0);
			const VertexData& vertex1 = vertices_->at(index1);
			const VertexData& vertex2 = vertices_->at(index2);

			const Vector3 edge1 = vertex1.position - vertex0.position;
			const Vector3 edge2 = vertex2.position - vertex0.position;
			const Vector2 deltaUV1 = vertex1.uv - vertex0.uv;
			const Vector2 deltaUV2 = vertex2.uv - vertex0.uv;

			const float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
			if (mathAbs(determinant) <= SMALLER_EPSILON)
			{
				continue;
			}

			const float inverseDeterminant = 1.f / determinant;
			const Vector3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * inverseDeterminant;
			const Vector3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * inverseDeterminant;

			if (tangent.ContainsNanOrInf() || bitangent.ContainsNanOrInf())
			{
				continue;
			}

			tangentAccumulators[index0] += tangent;
			tangentAccumulators[index1] += tangent;
			tangentAccumulators[index2] += tangent;

			bitangentAccumulators[index0] += bitangent;
			bitangentAccumulators[index1] += bitangent;
			bitangentAccumulators[index2] += bitangent;
		}
	}

	for (size_t vertexIndex = 0; vertexIndex < vertices_->size(); ++vertexIndex)
	{
		VertexData& vertex = vertices_->at(vertexIndex);
		const Vector3 normal = GetSafeNormal(vertex.normal);
		const Vector3 storedTangent(vertex.tangent.x, vertex.tangent.y, vertex.tangent.z);
		const bool hasStoredTangent = SMALLER_EPSILON < storedTangent.SquareLength();

		const Vector3 tangentSource = hasStoredTangent ? storedTangent : tangentAccumulators[vertexIndex];
		const Vector3 tangent = OrthogonalizeTangent(normal, tangentSource);

		float tangentSign = vertex.tangent.w < 0.f ? -1.f : 1.f;
		if (!hasStoredTangent || mathAbs(vertex.tangent.w) <= SMALLER_EPSILON)
		{
			const Vector3& bitangent = bitangentAccumulators[vertexIndex];
			if (SMALLER_EPSILON < bitangent.SquareLength())
			{
				tangentSign = Vector3::Cross(normal, tangent).Dot(bitangent) < 0.f ? -1.f : 1.f;
			}
		}

		vertex.tangent = Vector4(tangent, tangentSign);
	}
}
