#ifndef __MESH_CONTAINER_H__
#define __MESH_CONTAINER_H__

#include "Goknar/Core.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Geometry/Box.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Model/DynamicMeshLOD.h"
#include "Goknar/Model/SkeletalMeshLOD.h"
#include "Goknar/Model/StaticMeshLOD.h"
#include "Goknar/Model/InstancedStaticMeshLOD.h"
#include "Goknar/Renderer/Renderer.h"

#include <vector>

#include "MeshGeometry.h"

template<typename T>
struct GOKNAR_API LODSetting
{
	T* mesh{ nullptr };
	float frameCoverage{ MAX_FLOAT };
};

template<typename T>
class GOKNAR_API Mesh : public Content
{
public:
	Mesh();
	virtual ~Mesh();

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

class GOKNAR_API DynamicMesh : public Mesh<DynamicMeshLOD>
{
};

class GOKNAR_API SkeletalMesh : public Mesh<SkeletalMeshLOD>
{
};

class GOKNAR_API StaticMesh : public Mesh<StaticMeshLOD>
{
};

class GOKNAR_API InstancedStaticMesh : public Mesh<InstancedStaticMeshLOD>
{
};

template<typename T>
Mesh<T>::Mesh()
{
}

template<typename T>
Mesh<T>::~Mesh()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		delete LODIterator->mesh;
	}

	LODs_.clear();
}

template<typename T>
void Mesh<T>::AddLOD(const LODSetting<T>& LOD)
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
inline T* Mesh<T>::GetLOD(int index) const
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return nullptr;
	}

	return LODs_[index].mesh;
}

template<typename T>
inline float Mesh<T>::GetLODFrameCoverage(int index) const
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return 0.f;
	}

	return LODs_[index].frameCoverage;
}

template<typename T>
inline void Mesh<T>::SetLODFrameCoverage(int index, float frameCoverage)
{
	GOKNAR_CORE_ASSERT(0 <= index && index < (int)LODs_.size());

	if (index < 0 || (int)LODs_.size() <= index)
	{
		return;
	}

	LODs_[index].frameCoverage = frameCoverage;
}

template<typename T>
inline size_t Mesh<T>::GetLODIndex(float coverage) const
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
void Mesh<T>::PreInit()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->PreInit();
	}
}

template<typename T>
void Mesh<T>::Init()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->Init();
	}
}

template<typename T>
void Mesh<T>::PostInit()
{
	auto LODIterator = LODs_.begin();
	for (; LODIterator != LODs_.end(); ++LODIterator)
	{
		(*LODIterator).mesh->PostInit();
	}
}

#endif
