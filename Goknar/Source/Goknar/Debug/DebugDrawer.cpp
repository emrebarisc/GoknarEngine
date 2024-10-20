#include "pch.h"

#include "DebugDrawer.h"

#include "Color.h"
#include "Engine.h"
#include "ObjectBase.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Math/GoknarMath.h"
#include "Model/StaticMeshInstance.h"
#include "Physics/PhysicsUtils.h"

#include "Physics/Components/BoxCollisionComponent.h"
#include "Physics/Components/CapsuleCollisionComponent.h"
#include "Physics/Components/SphereCollisionComponent.h"
#include "Physics/Components/MovingTriangleMeshCollisionComponent.h"
#include "Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

StaticMesh* DebugDrawer::lineMesh_ = nullptr;
StaticMesh* DebugDrawer::arrowHeadMesh_ = nullptr;

DebugDrawer::DebugDrawer()
{
#ifdef GOKNAR_BUILD_DEBUG
	lineMesh_ = engine->GetResourceManager()->GetEngineContent<StaticMesh>("Debug/Meshes/SM_Line.fbx");
	arrowHeadMesh_ = engine->GetResourceManager()->GetEngineContent<StaticMesh>("Debug/Meshes/SM_ArrowHead.fbx");
#endif
}

DebugDrawer::~DebugDrawer()
{

}

void DebugDrawer::DrawLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
#ifdef GOKNAR_BUILD_DEBUG
	DebugObject* line = new DebugObject();
	StaticMeshComponent* lineStaticMeshComponent = line->AddSubComponent<StaticMeshComponent>();
	lineStaticMeshComponent->SetMesh(lineMesh_);

	StaticMeshInstance* lineMeshInstance = lineStaticMeshComponent->GetMeshInstance();
	MaterialInstance* materialInstance = MaterialInstance::Create(lineMesh_->GetMaterial());
	materialInstance->SetBaseColor(color.ToVector4());
	lineMeshInstance->SetMaterial(materialInstance);
	lineMeshInstance->SetIsCastingShadow(false);

	Vector3 endToStart = end - start;

	line->SetWorldScaling(Vector3{ endToStart.Length(), thickness, thickness }, false);
	line->SetWorldPosition(start, false);
	line->SetWorldRotation(Quaternion::FromTwoVectors(start, end));

	line->SetParent(owner);
#endif
}

void DebugDrawer::DrawArrow(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
#ifdef GOKNAR_BUILD_DEBUG
	DebugObject* arrow = new DebugObject();

	DebugObject* arrowHead = new DebugObject();
	StaticMeshComponent* arrowHeadStaticMeshComponent = arrowHead->AddSubComponent<StaticMeshComponent>();
	arrowHeadStaticMeshComponent->SetMesh(arrowHeadMesh_);

	StaticMeshInstance* arrowHeadMeshInstance = arrowHeadStaticMeshComponent->GetMeshInstance();
	MaterialInstance* materialInstance = MaterialInstance::Create(arrowHeadMesh_->GetMaterial());
	materialInstance->SetBaseColor(color.ToVector4());
	arrowHeadMeshInstance->SetMaterial(materialInstance);
	arrowHeadMeshInstance->SetIsCastingShadow(false);

	Vector3 endToStart = end - start;

	arrowHead->SetWorldScaling(Vector3{ thickness }, false);
	arrowHead->SetWorldPosition(end, false);
	arrowHead->SetWorldRotation(Quaternion::FromTwoVectors(start, end));

	arrowHead->SetParent(arrow);

	DrawLine(start, end, color, thickness, time, arrow);

	arrow->SetParent(owner);
#endif
}

void DebugDrawer::DrawCircle(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* circle = new DebugObject();

	float angleStep = 0.39269908169f;
	for (float angle = 0.f; angle < TWO_PI; angle += angleStep)
	{
		Vector3 start = Vector3{ radius * cosf(angle), radius * sinf(angle), 0.f };
		Vector3 end = Vector3{ radius * cosf(angle + angleStep), radius * sinf(angle + angleStep), 0.f };
		DrawLine(start, end, color, thickness, time, circle);
	}

	circle->SetWorldPosition(position, false);
	circle->SetWorldRotation(rotation);
	circle->SetParent(owner);
	circle->SetName("DebugObject_Circle");
}

void DebugDrawer::DrawSphere(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* sphere = new DebugObject();

	Matrix rotationMatrix = rotation.GetMatrix();

	Vector3 up = rotationMatrix.GetUpVector();
	Vector3 forward = rotationMatrix.GetForwardVector();

	float minTh = -HALF_PI;
	float maxTh = HALF_PI;
	float minPs = -PI;
	float maxPs = PI;
	float stepDegrees = 30.f;

	DrawSpherePatch(position, up, forward, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, time, sphere);

	sphere->SetName("DebugObject_DebugSphere");
	sphere->SetParent(owner);
}

void DebugDrawer::DrawBox(const Vector3& position, const Quaternion& rotation, const Vector3& halfSize, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* box = new DebugObject();

	Matrix rotationMatrix = rotation.GetMatrix();

	Vector3 forwardVector = rotationMatrix * Vector4(halfSize.x, 0.f, 0.f, 1.f);
	Vector3 leftVector = rotationMatrix * Vector4(0.f, halfSize.y, 0.f, 1.f);
	Vector3 upVector = rotationMatrix * Vector4(0.f, 0.f, halfSize.z, 1.f);

	Vector3 corners[8] =
	{
		{ position - forwardVector - leftVector - upVector },
		{ position + forwardVector - leftVector - upVector },
		{ position - forwardVector + leftVector - upVector },
		{ position + forwardVector + leftVector - upVector },
		{ position - forwardVector - leftVector + upVector },
		{ position + forwardVector - leftVector + upVector },
		{ position - forwardVector + leftVector + upVector },
		{ position + forwardVector + leftVector + upVector }
	};

	DrawLine(corners[0], corners[1], color, thickness, time, box);
	DrawLine(corners[0], corners[2], color, thickness, time, box);
	DrawLine(corners[3], corners[1], color, thickness, time, box);
	DrawLine(corners[3], corners[2], color, thickness, time, box);
	DrawLine(corners[4], corners[5], color, thickness, time, box);
	DrawLine(corners[4], corners[6], color, thickness, time, box);
	DrawLine(corners[7], corners[5], color, thickness, time, box);
	DrawLine(corners[7], corners[6], color, thickness, time, box);
	DrawLine(corners[0], corners[4], color, thickness, time, box);
	DrawLine(corners[1], corners[5], color, thickness, time, box);
	DrawLine(corners[2], corners[6], color, thickness, time, box);
	DrawLine(corners[3], corners[7], color, thickness, time, box);

	box->SetName("DebugObject_DebugBox");
	box->SetParent(owner);
}

void DebugDrawer::DrawCapsule(const Vector3& position, const Quaternion& rotation,
	float radius, float height, const Colorf& color,
	float thickness/* = 1.f*/, float time/* = -1.f*/,
	ObjectBase* owner/* = nullptr*/)
{
	DebugObject* capsule = new DebugObject();

	int stepDegrees = 30;
	float halfHeight = height * 0.5f;

	Vector3 capStart(0.f, 0.f, -halfHeight);
	Vector3 capEnd(0.f, 0.f, halfHeight);

	const float minTh = -HALF_PI;
	const float maxTh = HALF_PI;
	const float minPs = -PI;
	const float maxPs = PI;

	const Matrix rotationMatrix = rotation.GetMatrix();
	const Vector3 up = rotationMatrix.GetUpVector();
	const Vector3 forward = rotationMatrix.GetForwardVector();

	{
		Vector3 center = position - halfHeight * up;
		DrawSpherePatch(center, up, forward, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, time, capsule);
	}

	{
		Vector3 center = position + halfHeight * up;
		DrawSpherePatch(center, up, forward, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, time, capsule);
	}

	for (int i = 0; i < 360; i += stepDegrees)
	{
		capEnd.x = capStart.x = GoknarMath::Sin(i * TO_RADIAN) * radius;
		capEnd.y = capStart.y = GoknarMath::Cos(i * TO_RADIAN) * radius;

		DrawLine(
			position + rotationMatrix * Vector4{ capStart, 1.f },
			position + rotationMatrix * Vector4{ capEnd, 1.f },
			color, thickness, time, capsule);
	}

	capsule->SetName("DebugObject_Capsule");
	capsule->SetParent(owner);
}

void DebugDrawer::DrawCollisionComponent(const BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawBox(boxCollisionComponent->GetWorldPosition(), boxCollisionComponent->GetWorldRotation(), boxCollisionComponent->GetHalfSize() * boxCollisionComponent->GetWorldScaling(), color, thickness, time, boxCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawCapsule(
		capsuleCollisionComponent->GetWorldPosition(), capsuleCollisionComponent->GetWorldRotation(),
		capsuleCollisionComponent->GetRadius(), capsuleCollisionComponent->GetHeight(),
		color, thickness, time, capsuleCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawSphere(sphereCollisionComponent->GetWorldPosition(), sphereCollisionComponent->GetWorldRotation(), sphereCollisionComponent->GetRadius(), color, thickness, time, sphereCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent, const Colorf& color, float thickness, float time)
{
	DebugObject* collisionObject = new DebugObject();
	collisionObject->SetName("DebugObject_TriangleMeshCollisionComponent");

	const MeshUnit* mesh = movingTriangleMeshCollisionComponent->GetMesh();
	DrawMeshUnit(mesh, color, thickness, time, collisionObject);

	collisionObject->SetWorldPosition(movingTriangleMeshCollisionComponent->GetWorldPosition());
	collisionObject->SetWorldRotation(movingTriangleMeshCollisionComponent->GetWorldRotation());
	collisionObject->SetWorldScaling(movingTriangleMeshCollisionComponent->GetWorldScaling());

	collisionObject->SetParent(movingTriangleMeshCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent, const Colorf& color, float thickness, float time)
{
	DebugObject* collisionObject = new DebugObject();
	collisionObject->SetName("DebugObject_TriangleMeshCollisionComponent");

	const MeshUnit* mesh = nonMovingTriangleMeshCollisionComponent->GetMesh();
	DrawMeshUnit(mesh, color, thickness, time, collisionObject);

	collisionObject->SetWorldPosition(nonMovingTriangleMeshCollisionComponent->GetWorldPosition());
	collisionObject->SetWorldRotation(nonMovingTriangleMeshCollisionComponent->GetWorldRotation());
	collisionObject->SetWorldScaling(nonMovingTriangleMeshCollisionComponent->GetWorldScaling());

	collisionObject->SetParent(nonMovingTriangleMeshCollisionComponent->GetOwner());
}

void DebugDrawer::DrawMeshUnit(const MeshUnit* meshUnit, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	const VertexArray* vertexArray = meshUnit->GetVerticesPointer();
	int vertexCount = vertexArray->size();

	const FaceArray* faceArray = meshUnit->GetFacesPointer();
	int faceCount = faceArray->size();

	for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
	{
		const Face& face = faceArray->at(faceIndex);
		DrawTriangle(
			vertexArray->at(face.vertexIndices[0]).position,
			vertexArray->at(face.vertexIndices[1]).position,
			vertexArray->at(face.vertexIndices[2]).position,
			color, thickness, time, owner);
	}
}

void DebugDrawer::DrawSpherePatch(const Vector3& center, const Vector3& up, const Vector3& forward, float radius,
	float minTh, float maxTh, float minPs, float maxPs, const Colorf& color,
	float stepDegrees/* = 10.f*/, bool drawCenter/* = true*/,
	float thickness/* = 1.f*/, float time/* = -1.f*/,
	ObjectBase* owner/* = nullptr*/)
{
	Vector3 vA[74];
	Vector3 vB[74];
	Vector3* pvA = vA, * pvB = vB, * pT;
	Vector3 npole = center + up * radius;
	Vector3 spole = center - up * radius;
	Vector3 arcStart;
	float step = stepDegrees * TO_RADIAN;
	const Vector3& kv = up;
	const Vector3& iv = forward;
	Vector3 jv = kv.Cross(iv);
	bool drawN = false;
	bool drawS = false;
	if (minTh <= -HALF_PI)
	{
		minTh = -HALF_PI + step;
		drawN = true;
	}
	if (maxTh >= HALF_PI)
	{
		maxTh = HALF_PI - step;
		drawS = true;
	}
	if (minTh > maxTh)
	{
		minTh = -HALF_PI + step;
		maxTh = HALF_PI - step;
		drawN = drawS = true;
	}
	int n_hor = (int)((maxTh - minTh) / step) + 1;
	if (n_hor < 2) n_hor = 2;
	float step_h = (maxTh - minTh) / float(n_hor - 1);
	bool isClosed = false;
	if (minPs > maxPs)
	{
		minPs = -PI + step;
		maxPs = PI;
		isClosed = true;
	}
	else if ((maxPs - minPs) >= PI * float(2.f))
	{
		isClosed = true;
	}
	else
	{
		isClosed = false;
	}
	int n_vert = (int)((maxPs - minPs) / step) + 1;
	if (n_vert < 2) n_vert = 2;
	float step_v = (maxPs - minPs) / float(n_vert - 1);
	for (int i = 0; i < n_hor; i++)
	{
		float th = minTh + i * step_h;
		float sth = radius * GoknarMath::Sin(th);
		float cth = radius * GoknarMath::Cos(th);
		for (int j = 0; j < n_vert; j++)
		{
			float psi = minPs + float(j) * step_v;
			float sps = GoknarMath::Sin(psi);
			float cps = GoknarMath::Cos(psi);
			pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
			if (i)
			{
				DrawLine(pvA[j], pvB[j], color, thickness, time, owner);
			}
			else if (drawS)
			{
				DrawLine(spole, pvB[j], color, thickness, time, owner);
			}
			if (j)
			{
				DrawLine(pvB[j - 1], pvB[j], color, thickness, time, owner);
			}
			else
			{
				arcStart = pvB[j];
			}
			if ((i == (n_hor - 1)) && drawN)
			{
				DrawLine(npole, pvB[j], color, thickness, time, owner);
			}

			if (drawCenter)
			{
				if (isClosed)
				{
					if (j == (n_vert - 1))
					{
						DrawLine(arcStart, pvB[j], color, thickness, time, owner);
					}
				}
				else
				{
					if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1))))
					{
						DrawLine(center, pvB[j], color, thickness, time, owner);
					}
				}
			}
		}
		pT = pvA;
		pvA = pvB;
		pvB = pT;
	}
}

void DebugDrawer::DrawTriangle(const Vector3& position1, const Vector3& position2, const Vector3& position3, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DrawLine(position1, position2, color, thickness, time, owner);
	DrawLine(position2, position3, color, thickness, time, owner);
}
