#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "Component.h"

#include "Goknar/Core.h"

class Mesh;
class MeshInstance;
class ObjectBase;

class GOKNAR_API MeshComponent : public Component
{
public:
	MeshComponent() = delete;
	MeshComponent(ObjectBase* parent);
	~MeshComponent();

	void SetMesh(Mesh* mesh);

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	const Matrix& GetRerlativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

protected:
	inline void UpdateRelativeTransformationMatrix() override;

private:
	MeshInstance* meshInstance_;
};
#endif