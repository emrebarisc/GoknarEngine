#ifndef __MESHINSTANCE_H__
#define __MESHINSTANCE_H__

#include "Goknar/Core.h"
#include "Goknar/Matrix.h"

class Component;
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
		componentId_(lastComponentId_++)
	{
	}

	int GetComponentId() const
	{
		return componentId_;
	}

	void SetComponentId(int componentId)
	{
		componentId_ = componentId;
	}

	void SetMesh(Mesh* mesh);

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

	void Render() const;

protected:

private:
	Matrix relativeTransformationMatrix_, worldTransformationMatrix_;

	Component* parentComponent_;
	Mesh* mesh_;

	int componentId_;

	static int lastComponentId_;
};

#endif