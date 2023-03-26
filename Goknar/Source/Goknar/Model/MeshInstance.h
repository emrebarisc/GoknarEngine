#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

class RenderComponent;
class Material;
class MeshUnit;

class GOKNAR_API MeshInstance
{
public:
	MeshInstance() = delete;

	MeshInstance(RenderComponent* parentComponent) :
		parentComponent_(parentComponent)
	{
	}

	virtual ~MeshInstance() {}

	int GetComponentId() const
	{
		return componentId_;
	}

	void SetComponentId(int componentId)
	{
		componentId_ = componentId;
	}

	virtual void SetMesh(MeshUnit* mesh);

	MeshUnit* GetMesh() const
	{
		return mesh_;
	}

	void SetRelativeTransformationMatrix(const Matrix& relativeTransformationMatrix)
	{
		relativeTransformationMatrix_ = relativeTransformationMatrix;
	}

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void SetWorldTransformationMatrix(const Matrix& worldTransformationMatrix)
	{
		worldTransformationMatrix_ = worldTransformationMatrix;
	}

	const Matrix& GetWorldTransformationMatrix() const
	{
		return worldTransformationMatrix_;
	}

	virtual void Render();

	void SetIsRendered(bool isRendered)
	{
		isRendered_ = isRendered;
	}

	bool GetIsRendered() const
	{
		return isRendered_;
	}

	virtual void Destroy();

protected:
	virtual void AddMeshInstanceToRenderer() = 0;
	virtual void RemoveMeshInstanceFromRenderer() = 0;

	MeshUnit* mesh_{ nullptr };

private:
	Matrix relativeTransformationMatrix_{ Matrix::IdentityMatrix };
	Matrix worldTransformationMatrix_{ Matrix::IdentityMatrix };

	RenderComponent* parentComponent_{ nullptr };

	int componentId_{ lastComponentId_++ };

	static int lastComponentId_;

	bool isRendered_{ true };
};

#endif