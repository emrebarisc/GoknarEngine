#ifndef __DEBUGDRAWER_H__
#define __DEBUGDRAWER_H__

#include "Core.h"

#include "Color.h"
#include "Math/GoknarMath.h"

class BoxCollisionComponent;
class CapsuleCollisionComponent;
class SphereCollisionComponent;
class ObjectBase;
class StaticMesh;

class GOKNAR_API DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	static void DrawLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawBox(const Vector3& position, const Quaternion& rotation, const Vector3& halfSize, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawCapsule(const Vector3& position, float radius, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawCircle(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawHalfSphere(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawSphere(const Vector3& position, float radius, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);

	static void DrawCollisionComponent(BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawCollisionComponent(CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);
	static void DrawCollisionComponent(SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness = 1.f, float time = 1.f, ObjectBase* owner = nullptr);

protected:

private:
	static StaticMesh* lineMesh_;
};
#endif
