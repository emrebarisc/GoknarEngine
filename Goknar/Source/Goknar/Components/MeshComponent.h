#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "RenderComponent.h"

#include "Goknar/Core.h"
#include "Goknar/Model/MeshInstance.h"

class MeshUnit;
class ObjectBase;

class GOKNAR_API MeshComponent : public RenderComponent
{
public:
	MeshComponent() = delete;
	virtual ~MeshComponent();

	void Destroy() override;

	virtual void SetMesh(MeshUnit* mesh) = 0;

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

	void SetIsActive(bool isRendered);

	MeshInstance* GetMeshInstance() const
	{
		return meshInstance_;
	}
protected:
	MeshComponent(ObjectBase* parent, MeshInstance* meshInstance);
	inline void UpdateRelativeTransformationMatrix() override
	{
		RenderComponent::UpdateRelativeTransformationMatrix();

		meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
	}

	MeshInstance* meshInstance_;
private:
};
#endif