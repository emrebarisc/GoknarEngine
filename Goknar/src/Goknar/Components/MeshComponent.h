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
	virtual ~MeshComponent();

	void Destroy() override;

	virtual void SetMesh(Mesh* mesh) = 0;

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

	void SetIsRendered(bool isRendered);
protected:
	MeshComponent(ObjectBase* parent, MeshInstance* meshInstance);
	inline void UpdateRelativeTransformationMatrix() override;

	MeshInstance* meshInstance_;
private:
};
#endif