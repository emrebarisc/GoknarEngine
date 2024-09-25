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

DebugDrawer::DebugDrawer()
{
#ifdef GOKNAR_BUILD_DEBUG
	lineMesh_ = engine->GetResourceManager()->GetEngineContent<StaticMesh>("Debug/Meshes/SM_Line.fbx");
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

void DebugDrawer::DrawCircle(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* circle = new DebugObject();

	float angleStep = 0.39269908169f;
	for (float angle = 0.f; angle < TWO_PI; angle += angleStep)
	{
		Vector3 start = Vector3{ radius * cosf(angle), radius * sinf(angle), 0.f };
		Vector3 end = Vector3{ radius * cosf(angle + angleStep), radius * sinf(angle + angleStep), 0.f };
		DrawLine(start, end, color, thickness, -1.f, circle);
	}

	circle->SetWorldPosition(position, false);
	circle->SetWorldRotation(rotation, false);
	circle->SetParent(owner);
}

void DebugDrawer::DrawSphere(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* sphere = new DebugObject();

	Matrix rotationMatrix = rotation.GetMatrix();
	Vector3 up = rotationMatrix.GetUpVector();
	Vector3 axis = rotationMatrix.GetForwardVector();
	float minTh = -SIMD_HALF_PI;
	float maxTh = SIMD_HALF_PI;
	float minPs = -SIMD_HALF_PI;
	float maxPs = SIMD_HALF_PI;
	float stepDegrees = 30.f;

	DrawSpherePatch(position, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, -1.f, sphere);
	DrawSpherePatch(position, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, -1.f, sphere);

	sphere->SetName("DebugSphere");
	sphere->SetParent(owner, SnappingRule::None);
}

void DebugDrawer::DrawBox(const Vector3& position, const Quaternion& rotation, const Vector3& halfSize, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DebugObject* box = new DebugObject();

	Vector4 rotatedHalfSize = rotation.GetMatrix() * Vector4 { halfSize, 0.f };

	Matrix rotationMatrix = rotation.GetMatrix();
	Vector3 forwardVector = halfSize * rotationMatrix.GetForwardVector();
	Vector3 leftVector = halfSize * rotationMatrix.GetLeftVector();
	Vector3 upVector = halfSize * rotationMatrix.GetUpVector();

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

	DrawLine(corners[0], corners[1], color, thickness, -1.f, box);
	DrawLine(corners[0], corners[2], color, thickness, -1.f, box);

	DrawLine(corners[3], corners[1], color, thickness, -1.f, box);
	DrawLine(corners[3], corners[2], color, thickness, -1.f, box);

	DrawLine(corners[4], corners[5], color, thickness, -1.f, box);
	DrawLine(corners[4], corners[6], color, thickness, -1.f, box);

	DrawLine(corners[7], corners[5], color, thickness, -1.f, box);
	DrawLine(corners[7], corners[6], color, thickness, -1.f, box);

	DrawLine(corners[0], corners[4], color, thickness, -1.f, box);
	DrawLine(corners[1], corners[5], color, thickness, -1.f, box);
	DrawLine(corners[2], corners[6], color, thickness, -1.f, box);
	DrawLine(corners[3], corners[7], color, thickness, -1.f, box);

	box->SetName("DebugBox");
	box->SetParent(owner, SnappingRule::None);
}

void DebugDrawer::DrawCapsule(const Vector3& position, const Quaternion& rotation,
	float radius, float height, const Colorf& color,
	float thickness/* = 1.f*/, float time/* = -1.f*/, ObjectBase* owner/* = nullptr*/)
{
	DebugObject* capsule = new DebugObject();

	int stepDegrees = 30;

	float halfHeight = height * 0.5f;

	Vector3 capStart(0.f, 0.f, 0.f);
	capStart.z = -halfHeight;

	Vector3 capEnd(0.f, 0.f, 0.f);
	capEnd.z = halfHeight;

	Matrix rotationMatrix = rotation.GetMatrix();

	{
		Vector3 center = position + capStart;
		Vector3 up = rotationMatrix.GetForwardVector();
		Vector3 forward = -rotationMatrix.GetUpVector();
		float minTh = -HALF_PI;
		float maxTh = HALF_PI;
		float minPs = -HALF_PI;
		float maxPs = HALF_PI;

		DrawSpherePatch(center, up, forward, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, -1.f, capsule);
	}

	{
		Vector3 center = position + capEnd;
		Vector3 up = rotationMatrix.GetForwardVector();
		Vector3 forward = rotationMatrix.GetUpVector();
		float minTh = -HALF_PI;
		float maxTh = HALF_PI;
		float minPs = -HALF_PI;
		float maxPs = HALF_PI;
		DrawSpherePatch(center, up, forward, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false, thickness, -1.f, capsule);
	}

	for (int i = 0; i < 360; i += stepDegrees)
	{
		capEnd.x = capStart.x = GoknarMath::Sin(i * TO_RADIAN) * radius;
		capEnd.y = capStart.y = GoknarMath::Cos(i * TO_RADIAN) * radius;
		DrawLine(
			position + rotationMatrix * Vector4{ capStart, 1.f }, 
			position + rotationMatrix * Vector4{ capEnd, 1.f }, 
			color, thickness, -1.f, capsule);
	}

	capsule->SetName("DebugCapsule");
	capsule->SetParent(owner, SnappingRule::None);
}

void DebugDrawer::DrawCollisionComponent(const BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawBox(boxCollisionComponent->GetRelativePosition(), Quaternion::Identity, boxCollisionComponent->GetHalfSize() * boxCollisionComponent->GetRelativeScaling(), color, thickness, time, boxCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawCapsule(
		capsuleCollisionComponent->GetRelativePosition(), Quaternion::Identity,
		capsuleCollisionComponent->GetRadius(), capsuleCollisionComponent->GetHeight(),
		color, thickness, time, capsuleCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawSphere(sphereCollisionComponent->GetRelativePosition(), Quaternion::Identity, sphereCollisionComponent->GetRadius(), color, thickness, time, sphereCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(const MovingTriangleMeshCollisionComponent* movingTriangleMeshCollisionComponent, const Colorf& color, float thickness, float time)
{
	DebugObject* collisionObject = new DebugObject();
	collisionObject->SetName("DebugObject_TriangleMeshCollisionComponent");

	const MeshUnit* mesh = movingTriangleMeshCollisionComponent->GetMesh();
	DrawMeshUnit(mesh, color, thickness, time, collisionObject);

	collisionObject->SetWorldPosition(movingTriangleMeshCollisionComponent->GetRelativePosition());
	collisionObject->SetWorldRotation(movingTriangleMeshCollisionComponent->GetRelativeRotation());
	collisionObject->SetWorldScaling(movingTriangleMeshCollisionComponent->GetRelativeScaling());

	collisionObject->SetParent(movingTriangleMeshCollisionComponent->GetOwner(), SnappingRule::None);
}

void DebugDrawer::DrawCollisionComponent(const NonMovingTriangleMeshCollisionComponent* nonMovingTriangleMeshCollisionComponent, const Colorf& color, float thickness, float time)
{
	DebugObject* collisionObject = new DebugObject();
	collisionObject->SetName("DebugObject_TriangleMeshCollisionComponent");

	const MeshUnit* mesh = nonMovingTriangleMeshCollisionComponent->GetMesh();
	DrawMeshUnit(mesh, color, thickness, time, collisionObject);

	collisionObject->SetWorldPosition(nonMovingTriangleMeshCollisionComponent->GetRelativePosition());
	collisionObject->SetWorldRotation(nonMovingTriangleMeshCollisionComponent->GetRelativeRotation());
	collisionObject->SetWorldScaling(nonMovingTriangleMeshCollisionComponent->GetRelativeScaling());

	collisionObject->SetParent(nonMovingTriangleMeshCollisionComponent->GetOwner(), SnappingRule::None);
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
								 float thickness/* = 1.f*/, float time/* = -1.f*/, ObjectBase* owner/* = nullptr*/)
{
	Vector3 vA[74];
	Vector3 vB[74];
	Vector3 *pvA = vA, *pvB = vB, *pT;
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
		minPs = -SIMD_PI + step;
		maxPs = SIMD_PI;
		isClosed = true;
	}
	else if ((maxPs - minPs) >= SIMD_PI * float(2.f))
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
				DrawLine(pvA[j], pvB[j], color, thickness, -1.f, owner);
			}
			else if (drawS)
			{
				DrawLine(spole, pvB[j], color, thickness, -1.f, owner);
			}
			if (j)
			{
				DrawLine(pvB[j - 1], pvB[j], color, thickness, -1.f, owner);
			}
			else
			{
				arcStart = pvB[j];
			}
			if ((i == (n_hor - 1)) && drawN)
			{
				DrawLine(npole, pvB[j], color, thickness, -1.f, owner);
			}

			if (drawCenter)
			{
				if (isClosed)
				{
					if (j == (n_vert - 1))
					{
						DrawLine(arcStart, pvB[j], color, thickness, -1.f, owner);
					}
				}
				else
				{
					if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1))))
					{
						DrawLine(center, pvB[j], color, thickness, -1.f, owner);
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
