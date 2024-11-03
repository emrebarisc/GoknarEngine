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
class GOKNAR_API IMeshInstance
{
public:
	IMeshInstance() = delete;

	inline IMeshInstance(RenderComponent* parentComponent) :
		parentComponent_(parentComponent)
	{
	}

	inline virtual ~IMeshInstance()
	{
	}

	inline void PreInit();
	inline void Init();
	inline void PostInit();

	const RenderComponent* GetParentComponent() const
	{
		return parentComponent_;
	}

	inline int GetInstanceID() const
	{
		return instanceID_;
	}

	inline void SetInstanceID(int instanceID)
	{
		instanceID_ = instanceID;
	}

	inline virtual void SetMesh(MeshType* mesh);

	inline MeshType* GetMesh() const
	{
		return mesh_;
	}

	inline void SetMaterial(MaterialInstance* material);
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

	inline void SetIsCastingShadow(bool isCastingShadow)
	{
		isCastingShadow_ = isCastingShadow;
	}

	inline bool GetIsCastingShadow() const
	{
		return isCastingShadow_;
	}

	unsigned int GetRenderMask() const
	{
		return renderMask_;
	}

	void SetRenderMask(unsigned int renderMask)
	{
		renderMask_ = renderMask;
	}

	inline virtual void Destroy();

protected:
	virtual void AddMeshInstanceToRenderer() = 0;
	virtual void RemoveMeshInstanceFromRenderer() = 0;

	MeshType* mesh_{ nullptr };

	RenderComponent* parentComponent_{ nullptr };

	MaterialInstance* material_{ nullptr };
private:
	unsigned int renderMask_{ 0b1 };

	int instanceID_{ lastComponentId_++ };

	static int lastComponentId_;

	bool isRendered_{ true };
	bool isCastingShadow_{ true };
	bool isInitialized_{ false };
};

template<class MeshType>
int IMeshInstance<MeshType>::lastComponentId_ = 0;

template<class MeshType>
inline void IMeshInstance<MeshType>::PreInit()
{
	if (mesh_)
	{
		AddMeshInstanceToRenderer();
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::Init()
{
}

template<class MeshType>
inline void IMeshInstance<MeshType>::PostInit()
{
	isInitialized_ = true;
}

template<class MeshType>
inline void IMeshInstance<MeshType>::SetMaterial(MaterialInstance* material)
{
	bool refreshInstanceOnRenderer = false;

	if(material_ != nullptr)
	{
		refreshInstanceOnRenderer = material_->GetBlendModel() != material->GetBlendModel();
	}
	
	material_ = material;

	if(refreshInstanceOnRenderer)
	{
		RemoveMeshInstanceFromRenderer();
		AddMeshInstanceToRenderer();
	}
}

template<class MeshType>
inline IMaterialBase* IMeshInstance<MeshType>::GetMaterial()
{
	if (material_)
	{
		return material_;
	}

	if (mesh_)
	{
		return mesh_->GetMaterial();
	}

	return nullptr;
}

template<class MeshType>
inline void IMeshInstance<MeshType>::PreRender(RenderPassType renderPassType)
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
inline void IMeshInstance<MeshType>::Render(RenderPassType renderPassType)
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
inline void IMeshInstance<MeshType>::SetMesh(MeshType* mesh)
{
	if(mesh_ != nullptr)
	{
		RemoveMeshInstanceFromRenderer();
	}

	mesh_ = mesh;

	if (isInitialized_)
	{
		PreInit();
		Init();
		PostInit();
	}
}

template<class MeshType>
inline void IMeshInstance<MeshType>::Destroy()
{
	if (mesh_)
	{
		RemoveMeshInstanceFromRenderer();
	}
	delete this;
}

#endif