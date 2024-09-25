#ifndef __DEBUGDRAWER_H__
#define __DEBUGDRAWER_H__

#include "Core.h"

#include "Color.h"
#include "ObjectBase.h"
#include "Math/GoknarMath.h"

class BoxCollisionComponent;
class CapsuleCollisionComponent;
class SphereCollisionComponent;
class MovingTriangleMeshCollisionComponent;
class NonMovingTriangleMeshCollisionComponent;

class MeshUnit;
class ObjectBase;
class StaticMesh;

class GOKNAR_API DebugObject : public ObjectBase
{
public:
	DebugObject() : ObjectBase() {}
	~DebugObject() = default;
};

class GOKNAR_API DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	static void DrawLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);
	static void DrawBox(const Vector3& position, const Quaternion& rotation, const Vector3& halfSize, const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);
	static void DrawCapsule(const Vector3& position, const Quaternion& rotation, 
								float radius, float height, const Colorf& color, 
								float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawCircle(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);
	static void DrawSphere(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);
	static void DrawSpherePatch(const Vector3& center, const Vector3& up, const Vector3& axis, float radius,
								 float minTh, float maxTh, float minPs, float maxPs, const Colorf& color, 
								 float stepDegrees = float(10.f), bool drawCenter = true, 
								 float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);
	static void DrawTriangle(	const Vector3& position1, const Vector3& position2, const Vector3& position3, 
								const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);

	static void DrawCollisionComponent(const BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness = 1.f, float time = -1.f);
	static void DrawCollisionComponent(const CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness = 1.f, float time = -1.f);
	static void DrawCollisionComponent(const SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness = 1.f, float time = -1.f);
	static void DrawCollisionComponent(const MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent, const Colorf& color, float thickness = 1.f, float time = -1.f);
	static void DrawCollisionComponent(const NonMovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent, const Colorf& color, float thickness = 1.f, float time = -1.f);

	static void DrawMeshUnit(const MeshUnit* meshUnit, const Colorf& color, float thickness = 1.f, float time = -1.f, ObjectBase* owner = nullptr);

protected:

private:
	static StaticMesh* lineMesh_;
};
#endif
