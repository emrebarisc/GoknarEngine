#ifndef __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class StaticMesh;

class btTriangleMesh;

// For static(non-moving) collisions
class GOKNAR_API NonMovingTriangleMeshCollisionComponent : public CollisionComponent
{
public:
	NonMovingTriangleMeshCollisionComponent(Component* parent);
	NonMovingTriangleMeshCollisionComponent(ObjectBase* parentObjectBase);
	~NonMovingTriangleMeshCollisionComponent();

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
	btTriangleMesh* bulletTriangleMesh_;
};

#endif