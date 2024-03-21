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

	line->SetWorldScaling(Vector3{ start.Distance(end), thickness, thickness }, false);
	line->SetWorldPosition(start, false);
	line->SetWorldRotation(Quaternion::FromTwoVectors(start, end));

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
		DrawLine(start, end, color, thickness, time, circle);
	}

	circle->SetWorldPosition(position, false);
	circle->SetWorldRotation(rotation, false);
	circle->SetParent(owner);
}

void DebugDrawer::DrawHalfSphere(const Vector3& position, const Quaternion& rotation, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	ObjectBase* halfSphere = new ObjectBase();

	halfSphere->SetWorldPosition(position, false);
	halfSphere->SetWorldRotation(rotation, false);
	halfSphere->SetParent(owner);
}

void DebugDrawer::DrawSphere(const Vector3& position, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
}

void DebugDrawer::DrawBox(const Vector3& position, const Quaternion& rotation, const Vector3& halfSize, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	ObjectBase* box = new ObjectBase();

	Vector4 rotatedHalfSize = rotation.GetMatrix() * Vector4{ halfSize, 1.f };

	Vector3 corners[8] =
	{
		{ position.x - rotatedHalfSize.x, position.y - rotatedHalfSize.y, position.z - rotatedHalfSize.z },
		{ position.x + rotatedHalfSize.x, position.y - rotatedHalfSize.y, position.z - rotatedHalfSize.z },
		{ position.x - rotatedHalfSize.x, position.y + rotatedHalfSize.y, position.z - rotatedHalfSize.z },
		{ position.x + rotatedHalfSize.x, position.y + rotatedHalfSize.y, position.z - rotatedHalfSize.z },
		{ position.x - rotatedHalfSize.x, position.y - rotatedHalfSize.y, position.z + rotatedHalfSize.z },
		{ position.x + rotatedHalfSize.x, position.y - rotatedHalfSize.y, position.z + rotatedHalfSize.z },
		{ position.x - rotatedHalfSize.x, position.y + rotatedHalfSize.y, position.z + rotatedHalfSize.z },
		{ position.x + rotatedHalfSize.x, position.y + rotatedHalfSize.y, position.z + rotatedHalfSize.z } 
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

	box->SetName("DebugBox");
	box->SetParent(owner);
}

void DebugDrawer::DrawCapsule(const Vector3& position, float radius, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
}

void DebugDrawer::DrawCollisionComponent(BoxCollisionComponent* boxCollisionComponent, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DrawBox(boxCollisionComponent->GetWorldPosition(), boxCollisionComponent->GetWorldRotation(), boxCollisionComponent->GetHalfSize(), color, thickness, time, owner);
}

void DebugDrawer::DrawCollisionComponent(CapsuleCollisionComponent* capsuleCollisionComponent, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DrawCapsule(capsuleCollisionComponent->GetWorldPosition(), capsuleCollisionComponent->GetRadius(), color, thickness, time, owner);
}

void DebugDrawer::DrawCollisionComponent(SphereCollisionComponent* sphereCollisionComponent, const Colorf& color, float thickness, float time, ObjectBase* owner)
{
	DrawSphere(sphereCollisionComponent->GetWorldPosition(), sphereCollisionComponent->GetRadius(), color, thickness, time, owner);
}