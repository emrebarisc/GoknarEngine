#include "pch.h"

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"

#include "ObjectBase.h"
#include "StaticTriangleMeshCollisionComponent.h"
#include "Model/MeshUnit.h"
#include "Physics/PhysicsUtils.h"

#include "Log.h"

StaticTriangleMeshCollisionComponent::StaticTriangleMeshCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{

}

StaticTriangleMeshCollisionComponent::StaticTriangleMeshCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

StaticTriangleMeshCollisionComponent::~StaticTriangleMeshCollisionComponent()
{
}

void StaticTriangleMeshCollisionComponent::PreInit()
{
	GOKNAR_ASSERT(relativeMesh_ != nullptr);
	
	btTriangleMesh* bulletTriangleMesh = new btTriangleMesh(true, false);

	const Box& relativeMeshAABB = relativeMesh_->GetAABB();
	bulletTriangleMesh->setPremadeAabb(
		PhysicsUtils::FromVector3ToBtVector3(relativeMeshAABB.GetMin()),
		PhysicsUtils::FromVector3ToBtVector3(relativeMeshAABB.GetMax())
	);

	const VertexArray* vertexArray = relativeMesh_->GetVerticesPointer();

	const FaceArray* faceArray = relativeMesh_->GetFacesPointer();
	int faceCount = relativeMesh_->GetFaceCount();

	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		const Face& face = faceArray->at(faceIndex);
		bulletTriangleMesh->addTriangle(
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[0]).position),
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[1]).position),
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[2]).position)
			);
	}
	
	bulletCollisionShape_ = new btBvhTriangleMeshShape(bulletTriangleMesh, true, true);

	CollisionComponent::PreInit();
}

void StaticTriangleMeshCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void StaticTriangleMeshCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void StaticTriangleMeshCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void StaticTriangleMeshCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}