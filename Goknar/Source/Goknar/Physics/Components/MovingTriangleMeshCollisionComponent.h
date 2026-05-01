#ifndef __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class StaticMesh;

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

	const StaticMesh* GetMesh() const
	{
		return relativeMesh_;
	}

	void SetMesh(const StaticMesh* relativeMesh)
	{
		relativeMesh_ = relativeMesh;
	}

protected:
private:
	const StaticMesh* relativeMesh_{ nullptr };
};

#endif