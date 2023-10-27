#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

#include "Goknar/Components/RenderComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"

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

	inline virtual ~MeshInstance() {}

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

	inline virtual void PreRender(RenderPassType renderPassType = RenderPassType::Main);
	inline virtual void Render(RenderPassType renderPassType = RenderPassType::Main);

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
private:

	int componentId_{ lastComponentId_++ };

	static int lastComponentId_;

	bool isRendered_{ true };
};

template<class MeshType>
int MeshInstance<MeshType>::lastComponentId_ = 0;

template<class MeshType>
inline void MeshInstance<MeshType>::PreRender(RenderPassType renderPassType)
{
	mesh_->GetMaterial()->Use(renderPassType);
}

template<class MeshType>
inline void MeshInstance<MeshType>::Render(RenderPassType renderPassType)
{
	//
	//Matrix relativeTransformationMatrix = Matrix::IdentityMatrix;
	//Matrix worldTransformationMatrix = Matrix::IdentityMatrix;

	//// TODO: Optimize ////////
	//Component* parent = parentComponent_;
	//while (parent != nullptr)
	//{
	//	relativeTransformationMatrix = parent->GetRelativeTransformationMatrix() * relativeTransformationMatrix;
	//	parent = parent->GetParent();
	//}
	////////////////////////////
	//
	//if (parentComponent_)
	//{
	//	// Only read the upmost parent when owner objectbase is attached to another ObjectBase

	//	ObjectBase* parentObject = parentComponent_->GetOwner();
	//	while (parentObject->GetParent() != nullptr)
	//	{
	//		parentObject = parentObject->GetParent();
	//	}

	//	worldTransformationMatrix = parentObject->GetWorldTransformationMatrix();
	//}
	//else
	//{
	//	GOKNAR_FATAL("NULL PARENT ON RENDERING MESH INSTANCE");
	//}
	
	//mesh_->GetMaterial()->SetShaderVariables(worldTransformationMatrix, relativeTransformationMatrix);
	mesh_->GetMaterial()->SetShaderVariables(parentComponent_->GetComponentToWorldTransformationMatrix(), Matrix::IdentityMatrix, renderPassType);// worldTransformationMatrix, relativeTransformationMatrix);
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