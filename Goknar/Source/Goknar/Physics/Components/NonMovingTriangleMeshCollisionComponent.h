#ifndef __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class MeshUnit;

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

	virtual void UpdateTransformation() override;

	const MeshUnit* GetMesh() const
	{
		return relativeMesh_;
	}

	void SetMesh(const MeshUnit* relativeMesh)
	{
		relativeMesh_ = relativeMesh;
	}

	bool GetIsScalable() const
	{
		return isScalable_;
	}

	void SetIsScalable(bool isScalable)
	{
		isScalable_ = isScalable;
	}

protected:
private:
	const MeshUnit* relativeMesh_{ nullptr };
	btTriangleMesh* bulletTriangleMesh_;

	bool isScalable_{ false };
};

#endif