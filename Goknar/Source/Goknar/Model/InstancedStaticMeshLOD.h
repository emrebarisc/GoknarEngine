#ifndef __INSTANCEDSTATICMESHLOD_H__
#define __INSTANCEDSTATICMESHLOD_H__

#include "StaticMeshLOD.h"

#include <string>
#include <vector>

class Material;

class GOKNAR_API InstancedStaticMeshLOD : public StaticMeshLOD
{
public:
	InstancedStaticMeshLOD();

	virtual ~InstancedStaticMeshLOD();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void AddMesh(MeshGeometry* meshUnit) override;

	static InstancedStaticMeshLOD* CreateFromStaticMesh(const StaticMeshLOD* sourceMesh, const std::string& path);

	void AddInstanceTransformation(const Matrix& instanceTransformationMatrix, bool recalculateAABB = true);
	void SetInstanceTransformations(const std::vector<Matrix>& instanceTransformationMatrices, bool recalculateAABB = true);
	void SetInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB = true);
	void UpdateInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix, bool recalculateAABB = true);
	void UpdateAllTransforms();
	void RecalculateAABB();

	const Box& GetAABB() const
	{
		return instancedAABB_;
	}

	const Box& GetSubMeshInstanceAABB(size_t subMeshIndex) const
	{
		if (subMeshIndex < subMeshInstanceAABBs_.size())
		{
			return subMeshInstanceAABBs_[subMeshIndex];
		}

		return GetSubMeshes()[subMeshIndex]->GetAABB();
	}

	bool HasPendingFullTransformUpload() const
	{
		return hasPendingFullTransformUpload_;
	}

	void ClearPendingFullTransformUpload()
	{
		hasPendingFullTransformUpload_ = false;
	}

	size_t GetInstanceCount() const
	{
		return instanceTransformationMatrices_.size();
	}

	const Matrix& GetInstanceTransformationAt(size_t index) const
	{
		return instanceTransformationMatrices_[index];
	}

	const std::vector<Matrix>& GetInstanceTransformationMatrices() const
	{
		return instanceTransformationMatrices_;
	}

	void SetSourceMeshPath(const std::string& sourceMeshPath)
	{
		sourceMeshPath_ = sourceMeshPath;
	}

	const std::string& GetSourceMeshPath() const
	{
		return sourceMeshPath_;
	}

private:
	static bool IsValidAABB(const Box& aabb);
	static void ExtendBoundsWithPoint(Box& bounds, bool& hasBounds, const Vector3& point);
	static void AddTransformedAABBToBounds(
		const Box& localAABB,
		const Matrix& transformationMatrix,
		Box& bounds,
		bool& hasBounds);
	static Material* CloneMaterialForInstancedStaticMesh(const Material* sourceMaterial);
	static MeshGeometry* CloneMeshGeometryForInstancedStaticMesh(const MeshGeometry* sourceMeshGeometry);

	std::vector<Matrix> instanceTransformationMatrices_;
	std::vector<Box> subMeshInstanceAABBs_;
	Box instancedAABB_{};
	bool hasPendingFullTransformUpload_{ false };
	std::string sourceMeshPath_{};
};

#endif
