#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Matrix.h"

class Component;
class Material;
class Mesh;

class GOKNAR_API MeshInstance
{
public:
	MeshInstance() = delete;

	MeshInstance(Component* parentComponent) :
		relativeTransformationMatrix_(Matrix::IdentityMatrix),
		worldTransformationMatrix_(Matrix::IdentityMatrix),
		parentComponent_(parentComponent),
		mesh_(nullptr),
		componentId_(lastComponentId_++),
		isRendered_(true)
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

	virtual void SetMesh(Mesh* mesh) = 0;

	Mesh* GetMesh() const
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

	void Render(Material* renderMaterial = nullptr) const;

	void SetIsRendered(bool isRendered)
	{
		isRendered_ = isRendered;
	}

	bool GetIsRendered() const
	{
		return isRendered_;
	}

	virtual void Destroy() = 0;

protected:
	virtual void AddMeshInstanceToRenderer() = 0;
	virtual void RemoveMeshInstanceFromRenderer() = 0;

	Mesh* mesh_;

private:
	Matrix relativeTransformationMatrix_;
	Matrix worldTransformationMatrix_;

	Component* parentComponent_;

	int componentId_;

	static int lastComponentId_;

	bool isRendered_;
};

#endif