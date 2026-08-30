#ifndef __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__
#define __NONMOVINGTRIANGLEMESHCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

<<<<<<< HEAD
class StaticMeshLOD;
=======
class StaticMesh;
>>>>>>> master

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
	btTriangleMesh* bulletTriangleMesh_;
};

#endif
