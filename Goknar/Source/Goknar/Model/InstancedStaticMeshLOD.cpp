#include "pch.h"

#include "InstancedStaticMeshLOD.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Renderer/Renderer.h"

InstancedStaticMeshLOD::InstancedStaticMeshLOD() :
	StaticMeshLOD()
{
}

InstancedStaticMeshLOD::~InstancedStaticMeshLOD()
{
}

void InstancedStaticMeshLOD::PreInit()
{
	MeshSection<MeshGeometry>::PreInit();

	engine->AddInstancedStaticMeshToRenderer(this);
}

void InstancedStaticMeshLOD::Init()
{
	StaticMeshLOD::Init();
}

void InstancedStaticMeshLOD::PostInit()
{
	StaticMeshLOD::PostInit();
}

void InstancedStaticMeshLOD::AddMesh(MeshGeometry* meshUnit)
{
	if (meshUnit)
	{
		meshUnit->SetMeshType(MeshType::InstancedStatic);
	}

	StaticMeshLOD::AddMesh(meshUnit);
	subMeshInstanceAABBs_.push_back(meshUnit ? meshUnit->GetAABB() : Box());

	if (!instanceTransformationMatrices_.empty())
	{
		RecalculateAABB();
	}
}

InstancedStaticMeshLOD* InstancedStaticMeshLOD::CreateFromStaticMesh(const StaticMeshLOD* sourceMesh, const std::string& path)
{
	if (!sourceMesh || path.empty())
	{
		return nullptr;
	}

	InstancedStaticMeshLOD* instancedStaticMesh = new InstancedStaticMeshLOD();
	instancedStaticMesh->SetPath(path);
	instancedStaticMesh->SetSourceMeshPath(sourceMesh->GetPath());

	const auto& sourceSubMeshes = sourceMesh->GetSubMeshes();
	for (const MeshGeometry* sourceSubMesh : sourceSubMeshes)
	{
		MeshGeometry* clonedMeshGeometry = CloneMeshGeometryForInstancedStaticMesh(sourceSubMesh);
		if (clonedMeshGeometry)
		{
			instancedStaticMesh->AddMesh(clonedMeshGeometry);
		}
	}

	return instancedStaticMesh;
}

void InstancedStaticMeshLOD::AddInstanceTransformation(const Matrix& instanceTransformationMatrix, bool recalculateAABB)
{
	instanceTransformationMatrices_.push_back(instanceTransformationMatrix);

	if (recalculateAABB)
	{
		RecalculateAABB();
	}
}

void InstancedStaticMeshLOD::SetInstanceTransformations(const std::vector<Matrix>& instanceTransformationMatrices, bool recalculateAABB)
{
	instanceTransformationMatrices_ = instanceTransformationMatrices;

	if (recalculateAABB)
	{
		RecalculateAABB();
	}
}

void InstancedStaticMeshLOD::SetInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB)
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

void InstancedStaticMeshLOD::UpdateInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB)
{
	SetInstanceTransformationAt(index, instanceTransformationMatrix, recalculateAABB);
}

void InstancedStaticMeshLOD::UpdateAllTransforms()
{
	hasPendingFullTransformUpload_ = true;

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->RefreshInstancedStaticMeshTransformations(this);
	}
}

void InstancedStaticMeshLOD::RecalculateAABB()
{
	const std::vector<MeshGeometry*>& subMeshes = GetSubMeshes();
	subMeshInstanceAABBs_.resize(subMeshes.size());
	instancedAABB_ = Box();

	bool hasInstancedAABB = false;
	for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size(); ++subMeshIndex)
	{
		MeshGeometry* subMesh = subMeshes[subMeshIndex];
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

bool InstancedStaticMeshLOD::IsValidAABB(const Box& aabb)
{
	const Vector3& min = aabb.GetMin();
	const Vector3& max = aabb.GetMax();
	return min.x <= max.x &&
		min.y <= max.y &&
		min.z <= max.z;
}

void InstancedStaticMeshLOD::ExtendBoundsWithPoint(Box& bounds, bool& hasBounds, const Vector3& point)
{
	if (!hasBounds)
	{
		bounds = Box(point, point);
		hasBounds = true;
		return;
	}

	bounds.ExtendWRTPoint(point, false);
}

void InstancedStaticMeshLOD::AddTransformedAABBToBounds(
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

Material* InstancedStaticMeshLOD::CloneMaterialForInstancedStaticMesh(const Material* sourceMaterial)
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

MeshGeometry* InstancedStaticMeshLOD::CloneMeshGeometryForInstancedStaticMesh(const MeshGeometry* sourceMeshGeometry)
{
	if (!sourceMeshGeometry)
	{
		return nullptr;
	}

	const VertexArray* sourceVertices = sourceMeshGeometry->GetVerticesPointer();
	const FaceArray* sourceFaces = sourceMeshGeometry->GetFacesPointer();
	if (!sourceVertices || !sourceFaces)
	{
		return nullptr;
	}

	MeshGeometry* clonedMeshGeometry = new MeshGeometry();
	clonedMeshGeometry->SetName(sourceMeshGeometry->GetName());
	clonedMeshGeometry->SetMaterial(CloneMaterialForInstancedStaticMesh(sourceMeshGeometry->GetMaterial()));
	clonedMeshGeometry->SetBaseVertex(sourceMeshGeometry->GetBaseVertex());
	clonedMeshGeometry->SetVertexStartingIndex(sourceMeshGeometry->GetVertexStartingIndex());

	for (const VertexData& vertexData : *sourceVertices)
	{
		clonedMeshGeometry->AddVertexData(vertexData);
	}

	for (const Face& face : *sourceFaces)
	{
		clonedMeshGeometry->AddFace(face);
	}

	return clonedMeshGeometry;
}
