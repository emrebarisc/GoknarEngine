#ifndef __INSTANCEDSTATICMESH_H__
#define __INSTANCEDSTATICMESH_H__

#include "StaticMesh.h"

#include <string>
#include <vector>

class GOKNAR_API InstancedStaticMesh : public StaticMesh
{
public:
	InstancedStaticMesh();

	virtual ~InstancedStaticMesh();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void AddMesh(MeshUnit* meshUnit) override;

	static InstancedStaticMesh* CreateFromStaticMesh(const StaticMesh* sourceMesh, const std::string& path);

	void AddInstanceTransformation(const Matrix& instanceTransformationMatrix);
	void SetInstanceTransformations(const std::vector<Matrix>& instanceTransformationMatrices);
	void SetInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix);
	void UpdateInstanceTransformationAt(size_t index, const Matrix& instanceTransformationMatrix);
	void UpdateAllTransforms();

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
	std::vector<Matrix> instanceTransformationMatrices_;
	bool hasPendingFullTransformUpload_{ false };
	std::string sourceMeshPath_{};
};

#endif
