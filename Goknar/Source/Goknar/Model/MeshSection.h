#ifndef __MESHSECTION_H__
#define __MESHSECTION_H__

#include "Goknar/Core.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/Geometry/Box.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Renderer/Renderer.h"

#include <vector>

#include "MeshGeometry.h"

template<typename T>
class GOKNAR_API MeshSection : public Content
{
public:
	MeshSection();
	virtual ~MeshSection();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void AddMesh(T* meshUnit);

	const T* GetMesh(int index) const
	{
		return subMeshes_[index];
	}

	const std::vector<T*>& GetSubMeshes() const
	{
		return subMeshes_;
	}

	const Box& GetAABB() const
	{
		return aabb_;
	}

	void Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix);

protected:

private:
	Box aabb_;

	std::vector<T*> subMeshes_;
};

template<typename T>
MeshSection<T>::MeshSection()
{
}

template<typename T>
MeshSection<T>::~MeshSection()
{
	auto subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		delete* subMeshIterator;
	}
	subMeshes_.clear();
}

template<typename T>
void MeshSection<T>::AddMesh(T* meshUnit)
{
	subMeshes_.push_back(meshUnit);

	aabb_.Combine(meshUnit->GetAABB(), true);
}

template<typename T>
void MeshSection<T>::PreInit()
{
	auto subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		(*subMeshIterator)->PreInit();
	}
}

template<typename T>
void MeshSection<T>::Init()
{
	auto subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		(*subMeshIterator)->Init();
	}
}

template<typename T>
void MeshSection<T>::PostInit()
{
	auto subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		(*subMeshIterator)->PostInit();
	}
}

template<typename T>
void MeshSection<T>::Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix)
{
	auto subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		Material* meshUnitMaterial = (*subMeshIterator)->GetMaterial();
		if (meshUnitMaterial)
		{
			meshUnitMaterial->Render(renderPassType, worldAndRelativeTransformationMatrix);
		}
	}
}

#endif
