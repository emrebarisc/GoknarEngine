#include "pch.h"

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"

#include "ObjectBase.h"
#include "NonMovingTriangleMeshCollisionComponent.h"
#include "Model/MeshUnit.h"
#include "Physics/PhysicsUtils.h"

NonMovingTriangleMeshCollisionComponent::NonMovingTriangleMeshCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{

}

NonMovingTriangleMeshCollisionComponent::NonMovingTriangleMeshCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

NonMovingTriangleMeshCollisionComponent::~NonMovingTriangleMeshCollisionComponent()
{
	delete bulletTriangleMesh_;
}

void NonMovingTriangleMeshCollisionComponent::PreInit()
{
	GOKNAR_ASSERT(relativeMesh_ != nullptr);

	bulletTriangleMesh_ = new btTriangleMesh(true, false);

	const Box& relativeMeshAABB = relativeMesh_->GetAABB();
	bulletTriangleMesh_->setPremadeAabb(
		PhysicsUtils::FromVector3ToBtVector3(relativeMeshAABB.GetMin()),
		PhysicsUtils::FromVector3ToBtVector3(relativeMeshAABB.GetMax())
	);

	const VertexArray* vertexArray = relativeMesh_->GetVerticesPointer();

	const FaceArray* faceArray = relativeMesh_->GetFacesPointer();
	int faceCount = relativeMesh_->GetFaceCount();

	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		const Face& face = faceArray->at(faceIndex);
		bulletTriangleMesh_->addTriangle(
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[0]).position),
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[2]).position),
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[1]).position)
			);
	}
	bulletCollisionShape_ = new btBvhTriangleMeshShape(bulletTriangleMesh_, true, true);

	CollisionComponent::PreInit();
}

void NonMovingTriangleMeshCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void NonMovingTriangleMeshCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void NonMovingTriangleMeshCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void NonMovingTriangleMeshCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}