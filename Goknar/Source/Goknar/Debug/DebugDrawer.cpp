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

StaticMesh* DebugDrawer::lineMesh_ = nullptr;

DebugDrawer::DebugDrawer()
{
	lineMesh_ = engine->GetResourceManager()->GetEngineContent<StaticMesh>("Debug/Meshes/SM_Line.fbx");
}

DebugDrawer::~DebugDrawer()
{

}

void DebugDrawer::DrawLine(const Vector3& start, const Vector3& end, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	ObjectBase* line = new ObjectBase();
	StaticMeshComponent* lineStaticMeshComponent = line->AddSubComponent<StaticMeshComponent>();
	lineStaticMeshComponent->SetMesh(lineMesh_);

	StaticMeshInstance* lineMeshInstance = lineStaticMeshComponent->GetMeshInstance();
	MaterialInstance* materialInstance = MaterialInstance::Create(lineMesh_->GetMaterial());
	materialInstance->SetDiffuseReflectance(color.ToVector4());
	lineMeshInstance->SetMaterial(materialInstance);
	lineMeshInstance->SetIsCastingShadow(false);

	Vector3 endToStart = end - start;

	line->SetWorldScaling(Vector3{ endToStart.Length(), thickness, thickness }, false);
	line->SetWorldPosition(start, false);
	line->SetWorldRotation(Quaternion::FromTwoVectors(Vector3::ForwardVector, endToStart));

	line->SetParent(owner);
}

void DebugDrawer::DrawCircle(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	ObjectBase* circle = new ObjectBase();

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
	ObjectBase* sphere = new ObjectBase();

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
	ObjectBase* box = new ObjectBase();

	Vector4 rotatedHalfSize = rotation.GetMatrix() * Vector4{ halfSize, 0.f };

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

void DebugDrawer::DrawCapsule(	const Vector3& position, const Quaternion& rotation, 
								float radius, float height, const Colorf& color, 
								float thickness/* = 1.f*/, float time/* = -1.f*/, ObjectBase* owner/* = nullptr*/)
{
	ObjectBase* capsule = new ObjectBase();

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

void DebugDrawer::DrawCollisionComponent(BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawBox(Vector3::ZeroVector, Quaternion::Identity, boxCollisionComponent->GetHalfSize(), color, thickness, time, boxCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawCapsule(
		Vector3::ZeroVector, Quaternion::Identity, 
		capsuleCollisionComponent->GetRadius(), capsuleCollisionComponent->GetHeight(), 
		color, thickness, time, capsuleCollisionComponent->GetOwner());
}

void DebugDrawer::DrawCollisionComponent(SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness, float time)
{
	DrawSphere(Vector3::ZeroVector, Quaternion::Identity, sphereCollisionComponent->GetRadius(), color, thickness, time, sphereCollisionComponent->GetOwner());
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