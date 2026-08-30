#ifndef __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class StaticMeshLOD;

class btTriangleMesh;

// For static(moving) mesh collisions
class GOKNAR_API MovingTriangleMeshCollisionComponent : public CollisionComponent
{
public:
	MovingTriangleMeshCollisionComponent(Component* parent);
	MovingTriangleMeshCollisionComponent(ObjectBase* parentObjectBase);
	~MovingTriangleMeshCollisionComponent();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;
	Component* Clone() const override;

	const StaticMeshLOD* GetMesh() const
	{
		return relativeMesh_;
	}

	void SetMesh(const StaticMeshLOD* relativeMesh)
	{
		relativeMesh_ = relativeMesh;
	}

protected:
private:
	const StaticMeshLOD* relativeMesh_{ nullptr };
};

#endif
