#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/RenderComponent.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialInstance.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/SkeletalMesh.h"

#include "Goknar/Renderer/Renderer.h"

template<class MeshType>
class GOKNAR_API MeshInstance
{
public:
	MeshInstance() = delete;

	inline MeshInstance(RenderComponent* parentComponent) :
		parentComponent_(parentComponent)
	{
	}

	inline virtual ~MeshInstance()
	{
	}

	inline void PreInit();
	inline void Init();
	inline void PostInit();

	inline int GetComponentId() const
	{
		return componentId_;
	}

	inline void SetComponentId(int componentId)
	{
		componentId_ = componentId;
	}

	inline virtual void SetMesh(MeshType* mesh);

	inline MeshType* GetMesh() const
	{
		return mesh_;
	}

	inline void SetMaterial(MaterialInstance* material)
	{
		material_ = material;
	}

	inline IMaterialBase* GetMaterial();

	inline virtual void PreRender(RenderPassType renderPassType = RenderPassType::Forward);
	inline virtual void Render(RenderPassType renderPassType = RenderPassType::Forward);

	inline void SetIsRendered(bool isRendered)
	{
		isRendered_ = isRendered;
	}

	inline bool GetIsRendered() const
	{
		return isRendered_;
	}

	inline virtual void Destroy();

protected:
	virtual void AddMeshInstanceToRenderer() = 0;
	virtual void RemoveMeshInstanceFromRenderer() = 0;

	MeshType* mesh_{ nullptr };

	RenderComponent* parentComponent_{ nullptr };

	MaterialInstance* material_{ nullptr };
private:

	int componentId_{ lastComponentId_++ };

	static int lastComponentId_;

	bool isRendered_{ true };
};

template<class MeshType>
int MeshInstance<MeshType>::lastComponentId_ = 0;

template<class MeshType>
inline void MeshInstance<MeshType>::PreInit()
{
}

template<class MeshType>
inline void MeshInstance<MeshType>::Init()
{
}

template<class MeshType>
inline void MeshInstance<MeshType>::PostInit()
{
}

template<class MeshType>
inline IMaterialBase* MeshInstance<MeshType>::GetMaterial()
{
	if (material_)
	{
		return material_;
	}

	return mesh_->GetMaterial();
}

template<class MeshType>
inline void MeshInstance<MeshType>::PreRender(RenderPassType renderPassType)
{
	if(material_)
	{
		material_->Use(renderPassType);
	}
	else
	{
		mesh_->GetMaterial()->Use(renderPassType);
	}
}

template<class MeshType>
inline void MeshInstance<MeshType>::Render(RenderPassType renderPassType)
{
	if(material_)
	{
		material_->SetShaderVariables(renderPassType, parentComponent_->GetComponentToWorldTransformationMatrix());
	}
	else
	{
		mesh_->GetMaterial()->SetShaderVariables(renderPassType, parentComponent_->GetComponentToWorldTransformationMatrix());
	}
}

template<class MeshType>
inline void MeshInstance<MeshType>::SetMesh(MeshType* mesh)
{
	mesh_ = mesh;
	AddMeshInstanceToRenderer();
}

template<class MeshType>
inline void MeshInstance<MeshType>::Destroy()
{
	RemoveMeshInstanceFromRenderer();
	delete this;
}

#endif