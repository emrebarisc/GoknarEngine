#ifndef __MESH_CONTAINER_H__
#define __MESH_CONTAINER_H__

#include "Goknar/Core.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Geometry/Box.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Renderer/Renderer.h"

#include <vector>

#include "MeshUnit.h"

template<typename T>
class MeshContainer;

using DynamicMeshContainer = MeshContainer<DynamicMesh>;
using SkeletalMeshContainer = MeshContainer<SkeletalMesh>;
using StaticMeshContainer = MeshContainer<StaticMesh>;
using InstancedStaticMeshContainer = MeshContainer<InstancedStaticMesh>;

template<typename T>
struct GOKNAR_API LODSetting
{
	T* mesh{ nullptr };
	float frameCoverage{ MAX_FLOAT };
};

template<typename T>
class GOKNAR_API MeshContainer : Content
{
public:
	MeshContainer();
	virtual ~MeshContainer();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void AddLOD(const LODSetting<T>& LOD);

	T* GetLOD(int index) const;
	size_t GetLODIndex(float coverage) const;

	const Box& GetAABB() const
	{
		return aabb_;
	}

	const std::vector<LODSetting<T>>& GetLODs() const
	{
		return LODs_;
	}

	size_t GetLODCount() const
	{
		return LODs_.size();
	}

protected:

private:
	Box aabb_;

	std::vector<LODSetting<T>> LODs_;
};

template<typename T>
MeshContainer<T>::MeshContainer()
{
}

template<typename T>
MeshContainer<T>::~MeshContainer()
{
	LODs_.clear();
}

template<typename T>
void MeshContainer<T>::AddLOD(const LODSetting<T>& LOD)
{
	size_t lodCount = LODs_.size();

	auto LODIterator = LODs_.cbegin();
	for (; LODIterator != LODs_.cend(); ++LODIterator)
	{
		if (LOD.frameCoverage < LODIterator->frameCoverage)
		{
			break;
		}
	}

	LODs_.insert(LODIterator, std::move(LOD));
	aabb_.Combine(LOD.mesh->GetAABB(), true);
}

template<typename T>
inline T* MeshContainer<T>::GetLOD(int index) const
{
	GOKNAR_CORE_ASSERT(index < LODs_.size());

	return LODs_[index].mesh;
}

template<typename T>
inline size_t MeshContainer<T>::GetLODIndex(float coverage) const
{
	size_t LODCount = LODs_.size();

	size_t result = 0;
	for (; result < LODCount; ++result)
	{
		if (coverage < LODIterator->distance)
		{
			break;
		}
	}

	return result;
}

template<typename T>
void MeshContainer<T>::PreInit()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->PreInit();
	}
}

template<typename T>
void MeshContainer<T>::Init()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->Init();
	}
}

template<typename T>
void MeshContainer<T>::PostInit()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->PostInit();
	}
}

#endif