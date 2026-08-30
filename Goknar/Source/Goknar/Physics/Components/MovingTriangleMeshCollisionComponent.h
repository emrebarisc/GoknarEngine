#ifndef __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __MOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

<<<<<<< HEAD
class StaticMeshLOD;
=======
class StaticMesh;
>>>>>>> master

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

<<<<<<< HEAD
	const StaticMeshLOD* GetMesh() const
=======
	const StaticMesh* GetMesh() const
>>>>>>> master
	{
		return relativeMesh_;
	}

<<<<<<< HEAD
	void SetMesh(const StaticMeshLOD* relativeMesh)
=======
	void SetMesh(const StaticMesh* relativeMesh)
>>>>>>> master
	{
		relativeMesh_ = relativeMesh;
	}

protected:
private:
<<<<<<< HEAD
	const StaticMeshLOD* relativeMesh_{ nullptr };
=======
	const StaticMesh* relativeMesh_{ nullptr };
>>>>>>> master
};

#endif
