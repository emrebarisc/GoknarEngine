#include "pch.h"

#include "InstancedStaticMesh.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Renderer/Renderer.h"

namespace
{
	Material* CloneMaterialForInstancedStaticMesh(const Material* sourceMaterial)
	{
		if (!sourceMaterial)
		{
			return nullptr;
		}

		Material* clonedMaterial = new Material();
		clonedMaterial->SetName(sourceMaterial->GetName());
		clonedMaterial->SetAmbientReflectance(sourceMaterial->GetAmbientReflectance());
		clonedMaterial->SetBaseColor(sourceMaterial->GetBaseColor());
		clonedMaterial->SetSpecularReflectance(sourceMaterial->GetSpecularReflectance());
		clonedMaterial->SetEmisiveColor(sourceMaterial->GetEmisiveColor());
		clonedMaterial->SetPhongExponent(sourceMaterial->GetPhongExponent());
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
			clonedInitializationData->emisiveColor = sourceInitializationData->emisiveColor;
			clonedInitializationData->fragmentNormal = sourceInitializationData->fragmentNormal;
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

	MeshUnit* CloneMeshUnitForInstancedStaticMesh(const MeshUnit* sourceMeshUnit)
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
}

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

	if (engine && engine->GetResourceManager())
	{
		engine->GetResourceManager()->GetResourceContainer()->AddMesh(instancedStaticMesh);
	}

	return instancedStaticMesh;
}

void InstancedStaticMesh::AddInstanceTransformation(const Matrix& instanceTransformationMatrix)
{
	instanceTransformationMatrices_.push_back(instanceTransformationMatrix);
}

void InstancedStaticMesh::SetInstanceTransformations(const std::vector<Matrix>& instanceTransformationMatrices)
{
	instanceTransformationMatrices_ = instanceTransformationMatrices;
}

void InstancedStaticMesh::SetInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix)
{
	GOKNAR_CORE_ASSERT(index < instanceTransformationMatrices_.size(), "InstancedStaticMesh instance index is out of bounds.");
	if (index >= instanceTransformationMatrices_.size())
	{
		return;
	}

	instanceTransformationMatrices_[index] = instanceTransformationMatrix;

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->UpdateInstancedStaticMeshTransformation(this, (int)index, instanceTransformationMatrix);
	}
}

void InstancedStaticMesh::UpdateInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix)
{
	SetInstanceTransformationAt(index, instanceTransformationMatrix);
}

void InstancedStaticMesh::UpdateAllTransforms()
{
	hasPendingFullTransformUpload_ = true;

	if (engine && engine->GetRenderer())
	{
		engine->GetRenderer()->RefreshInstancedStaticMeshTransformations(this);
	}
}
