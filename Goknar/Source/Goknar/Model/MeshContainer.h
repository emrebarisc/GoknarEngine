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
class GOKNAR_API MeshContainer : public Content
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
	float GetLODFrameCoverage(int index) const;
	void SetLODFrameCoverage(int index, float frameCoverage);

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
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		delete LODIterator->mesh;
	}

	LODs_.clear();
}

template<typename T>
void MeshContainer<T>::AddLOD(const LODSetting<T>& LOD)
{
	GOKNAR_CORE_ASSERT(LOD.mesh);

	if (!LOD.mesh)
	{
		return;
	}

	LODs_.push_back(LOD);
	aabb_.Combine(LOD.mesh->GetAABB(), true);
}

template<typename T>
inline T* MeshContainer<T>::GetLOD(int index) const
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return nullptr;
	}

	return LODs_[index].mesh;
}

template<typename T>
inline float MeshContainer<T>::GetLODFrameCoverage(int index) const
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return 0.f;
	}

	return LODs_[index].frameCoverage;
}

template<typename T>
inline void MeshContainer<T>::SetLODFrameCoverage(int index, float frameCoverage)
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return;
	}

	LODs_[index].frameCoverage = frameCoverage;
}

template<typename T>
inline size_t MeshContainer<T>::GetLODIndex(float coverage) const
{
	size_t LODCount = LODs_.size();

	if (LODCount == 0)
	{
		return 0;
	}

	size_t result = 0;
	for (size_t lodIndex = 1; lodIndex < LODCount; ++lodIndex)
	{
		if (coverage < LODs_[lodIndex].frameCoverage)
		{
			result = lodIndex;
		}
		else
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
