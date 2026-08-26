#include "pch.h"

#include "InstancedStaticMesh.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Renderer/Renderer.h"

InstancedStaticMesh::InstancedStaticMesh() :
	StaticMesh()
{
}

InstancedStaticMesh::~InstancedStaticMesh()
{
}

void InstancedStaticMesh::PreInit()
{
	Mesh<MeshUnit>::PreInit();

	engine->AddInstancedStaticMeshToRenderer(this);
}

void InstancedStaticMesh::Init()
{
	StaticMesh::Init();
}

void InstancedStaticMesh::PostInit()
{
	StaticMesh::PostInit();
}

void InstancedStaticMesh::AddMesh(MeshUnit* meshUnit)
{
	if (meshUnit)
	{
		meshUnit->SetMeshType(MeshType::InstancedStatic);
	}

	StaticMesh::AddMesh(meshUnit);
	subMeshInstanceAABBs_.push_back(meshUnit ? meshUnit->GetAABB() : Box());

	if (!instanceTransformationMatrices_.empty())
	{
		RecalculateAABB();
	}
}

InstancedStaticMesh* InstancedStaticMesh::CreateFromStaticMesh(const StaticMesh* sourceMesh, const std::string& path)
{
	if (!sourceMesh || path.empty())
	{
		return nullptr;
	}

	InstancedStaticMesh* instancedStaticMesh = new InstancedStaticMesh();
	instancedStaticMesh->SetPath(path);
	instancedStaticMesh->SetSourceMeshPath(sourceMesh->GetPath());

	const auto& sourceSubMeshes = sourceMesh->GetSubMeshes();
	for (const MeshUnit* sourceSubMesh : sourceSubMeshes)
	{
		MeshUnit* clonedMeshUnit = CloneMeshUnitForInstancedStaticMesh(sourceSubMesh);
		if (clonedMeshUnit)
		{
			instancedStaticMesh->AddMesh(clonedMeshUnit);
		}
	}

	return instancedStaticMesh;
}

void InstancedStaticMesh::AddInstanceTransformation(const Matrix& instanceTransformationMatrix, bool recalculateAABB)
{
	instanceTransformationMatrices_.push_back(instanceTransformationMatrix);

	if (recalculateAABB)
	{
		RecalculateAABB();
	}
}

void InstancedStaticMesh::SetInstanceTransformations(const std::vector<Matrix>& instanceTransformationMatrices, bool recalculateAABB)
{
	instanceTransformationMatrices_ = instanceTransformationMatrices;

	if (recalculateAABB)
	{
		RecalculateAABB();
	}
}

void InstancedStaticMesh::SetInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB)
{
	GOKNAR_CORE_ASSERT(index < instanceTransformationMatrices_.size(), "InstancedStaticMesh instance index is out of bounds.");
	if (index >= instanceTransformationMatrices_.size())
	{
		return;
	}

	instanceTransformationMatrices_[index] = instanceTransformationMatrix;

	if (recalculateAABB)
	{
		RecalculateAABB();
	}

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->UpdateInstancedStaticMeshTransformation(this, (int)index, instanceTransformationMatrix);
	}
}

void InstancedStaticMesh::UpdateInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB)
{
	SetInstanceTransformationAt(index, instanceTransformationMatrix, recalculateAABB);
}

void InstancedStaticMesh::UpdateAllTransforms()
{
	hasPendingFullTransformUpload_ = true;

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->RefreshInstancedStaticMeshTransformations(this);
	}
}

void InstancedStaticMesh::RecalculateAABB()
{
	const std::vector<MeshUnit*>& subMeshes = GetSubMeshes();
	subMeshInstanceAABBs_.resize(subMeshes.size());
	instancedAABB_ = Box();

	bool hasInstancedAABB = false;
	for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size(); ++subMeshIndex)
	{
		MeshUnit* subMesh = subMeshes[subMeshIndex];
		if (!subMesh || instanceTransformationMatrices_.empty())
		{
			subMeshInstanceAABBs_[subMeshIndex] = Box();
			continue;
		}

		Box subMeshInstanceAABB;
		bool hasSubMeshInstanceAABB = false;
		for (const Matrix& instanceTransformationMatrix : instanceTransformationMatrices_)
		{
			AddTransformedAABBToBounds(
				subMesh->GetAABB(),
				instanceTransformationMatrix,
				subMeshInstanceAABB,
				hasSubMeshInstanceAABB);
		}

		if (!hasSubMeshInstanceAABB)
		{
			subMeshInstanceAABBs_[subMeshIndex] = Box();
			continue;
		}

		subMeshInstanceAABB.CalculateSize();
		subMeshInstanceAABBs_[subMeshIndex] = subMeshInstanceAABB;

		if (!hasInstancedAABB)
		{
			instancedAABB_ = subMeshInstanceAABB;
			hasInstancedAABB = true;
		}
		else
		{
			instancedAABB_.Combine(subMeshInstanceAABB, false);
		}
	}

	if (hasInstancedAABB)
	{
		instancedAABB_.CalculateSize();
	}
}

bool InstancedStaticMesh::IsValidAABB(const Box& aabb)
{
	const Vector3& min = aabb.GetMin();
	const Vector3& max = aabb.GetMax();
	return min.x <= max.x &&
		min.y <= max.y &&
		min.z <= max.z;
}

void InstancedStaticMesh::ExtendBoundsWithPoint(Box& bounds, bool& hasBounds, const Vector3& point)
{
	if (!hasBounds)
	{
		bounds = Box(point, point);
		hasBounds = true;
		return;
	}

	bounds.ExtendWRTPoint(point, false);
}

void InstancedStaticMesh::AddTransformedAABBToBounds(
	const Box& localAABB,
	const Matrix& transformationMatrix,
	Box& bounds,
	bool& hasBounds)
{
	if (!IsValidAABB(localAABB))
	{
		return;
	}

	const Vector3& min = localAABB.GetMin();
	const Vector3& max = localAABB.GetMax();
	for (int xIndex = 0; xIndex < 2; ++xIndex)
	{
		for (int yIndex = 0; yIndex < 2; ++yIndex)
		{
			for (int zIndex = 0; zIndex < 2; ++zIndex)
			{
				const Vector3 corner(
					xIndex == 0 ? min.x : max.x,
					yIndex == 0 ? min.y : max.y,
					zIndex == 0 ? min.z : max.z);
				ExtendBoundsWithPoint(bounds, hasBounds, Vector3(transformationMatrix * Vector4(corner, 1.f)));
			}
		}
	}
}

Material* InstancedStaticMesh::CloneMaterialForInstancedStaticMesh(const Material* sourceMaterial)
{
	if (!sourceMaterial)
	{
		return nullptr;
	}

	Material* clonedMaterial = new Material();
	clonedMaterial->SetName(sourceMaterial->GetName());
	clonedMaterial->SetBaseColor(sourceMaterial->GetBaseColor());
	clonedMaterial->SetAmbientOcclusion(sourceMaterial->GetAmbientOcclusion());
	clonedMaterial->SetMetallic(sourceMaterial->GetMetallic());
	clonedMaterial->SetRoughness(sourceMaterial->GetRoughness());
	clonedMaterial->SetEmissiveColor(sourceMaterial->GetEmissiveColor());
	clonedMaterial->SetTranslucency(sourceMaterial->GetTranslucency());
	clonedMaterial->SetBlendModel(sourceMaterial->GetBlendModel());
	clonedMaterial->SetShadingModel(sourceMaterial->GetShadingModel());

	const std::vector<const Image*>* textureImages = sourceMaterial->GetTextureImages();
	if (textureImages)
	{
		for (const Image* textureImage : *textureImages)
		{
			if (textureImage)
			{
				clonedMaterial->AddTextureImage(textureImage);
			}
		}
	}

	const MaterialInitializationData* sourceInitializationData = sourceMaterial->GetInitializationData();
	MaterialInitializationData* clonedInitializationData = clonedMaterial->GetInitializationData();
	if (sourceInitializationData && clonedInitializationData)
	{
		clonedInitializationData->baseColor = sourceInitializationData->baseColor;
		clonedInitializationData->emissiveColor = sourceInitializationData->emissiveColor;
		clonedInitializationData->ambientOcclusion = sourceInitializationData->ambientOcclusion;
		clonedInitializationData->metallic = sourceInitializationData->metallic;
		clonedInitializationData->roughness = sourceInitializationData->roughness;
		clonedInitializationData->fragmentNormal = sourceInitializationData->fragmentNormal;
		clonedInitializationData->fragmentNormalIsTangentSpace = sourceInitializationData->fragmentNormalIsTangentSpace;
		clonedInitializationData->vertexNormal = sourceInitializationData->vertexNormal;
		clonedInitializationData->uv = sourceInitializationData->uv;
		clonedInitializationData->vertexPositionOffset = sourceInitializationData->vertexPositionOffset;
		clonedInitializationData->vertexShaderFunctions = sourceInitializationData->vertexShaderFunctions;
		clonedInitializationData->fragmentShaderFunctions = sourceInitializationData->fragmentShaderFunctions;
		clonedInitializationData->vertexShaderUniforms = sourceInitializationData->vertexShaderUniforms;
		clonedInitializationData->fragmentShaderUniforms = sourceInitializationData->fragmentShaderUniforms;
	}

	return clonedMaterial;
}

MeshUnit* InstancedStaticMesh::CloneMeshUnitForInstancedStaticMesh(const MeshUnit* sourceMeshUnit)
{
	if (!sourceMeshUnit)
	{
		return nullptr;
	}

	const VertexArray* sourceVertices = sourceMeshUnit->GetVerticesPointer();
	const FaceArray* sourceFaces = sourceMeshUnit->GetFacesPointer();
	if (!sourceVertices || !sourceFaces)
	{
		return nullptr;
	}

	MeshUnit* clonedMeshUnit = new MeshUnit();
	clonedMeshUnit->SetName(sourceMeshUnit->GetName());
	clonedMeshUnit->SetMaterial(CloneMaterialForInstancedStaticMesh(sourceMeshUnit->GetMaterial()));
	clonedMeshUnit->SetBaseVertex(sourceMeshUnit->GetBaseVertex());
	clonedMeshUnit->SetVertexStartingIndex(sourceMeshUnit->GetVertexStartingIndex());

	for (const VertexData& vertexData : *sourceVertices)
	{
		clonedMeshUnit->AddVertexData(vertexData);
	}

	for (const Face& face : *sourceFaces)
	{
		clonedMeshUnit->AddFace(face);
	}

	return clonedMeshUnit;
}
