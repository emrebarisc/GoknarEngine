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

	inline void SetRelativeTransformationMatrix(const Matrix& relativeTransformationMatrix)
	{
		relativeTransformationMatrix_ = relativeTransformationMatrix;
	}

	inline const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	inline void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix)
	{
		worldTransformationMatrix_ = worldTransformationMatrix;
	}

	inline const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	inline virtual void PreRender();
	inline virtual void Render();

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

private:
	Matrix relativeTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix worldTransformationMatrix_{ Matrix::IdentityMatrix };

	RenderComponent* parentComponent_{ nullptr };

	int componentId_{ lastComponentId_++ };

	static int lastComponentId_;

	bool isRendered_{ true };
};

template<class MeshType>
int MeshInstance<MeshType>::lastComponentId_ = 0;

template<class MeshType>
inline void MeshInstance<MeshType>::PreRender()
{
	mesh_->GetMaterial()->Use();
}

template<class MeshType>
inline void MeshInstance<MeshType>::Render()
{
	//Matrix relativeTransformationMatrix = relativeTransformationMatrix_;

	//// TODO: Optimize ////////
	//Component* parent = parentComponent_;
	//while (parent != nullptr)
	//{
	//	relativeTransformationMatrix *= parent->GetRelativeTransformationMatrix();
	//	parent = parent->GetParent();
	//}
	////////////////////////////
	mesh_->GetMaterial()->SetShaderVariables(worldTransformationMatrix_, relativeTransformationMatrix_);
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