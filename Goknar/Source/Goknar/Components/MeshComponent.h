#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "Component.h"

#include "Goknar/Core.h"
#include "Goknar/Model/MeshInstance.h"

class Mesh;
class ObjectBase;

class GOKNAR_API MeshComponent : public Component
{
public:
	MeshComponent() = delete;
	virtual ~MeshComponent();

	void Destroy() override;

	virtual void SetMesh(Mesh* mesh) = 0;

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

	void SetIsRendered(bool isRendered);

	MeshInstance* GetMeshInstance() const
	{
		return meshInstance_;
	}
protected:
	MeshComponent(ObjectBase* parent, MeshInstance* meshInstance);
	inline void UpdateRelativeTransformationMatrix() override
	{
		Component::UpdateRelativeTransformationMatrix();

		meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
	}

	MeshInstance* meshInstance_;
private:
};
#endif