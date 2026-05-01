#include "pch.h"

#include "BulletCollision/CollisionShapes/btConvexHullShape.h"

#include "GoknarAssert.h"
#include "ObjectBase.h"
#include "MovingTriangleMeshCollisionComponent.h"
#include "Model/StaticMesh.h"

MovingTriangleMeshCollisionComponent::MovingTriangleMeshCollisionComponent(Component* parent) :
	CollisionComponent(parent)
{

}

MovingTriangleMeshCollisionComponent::MovingTriangleMeshCollisionComponent(ObjectBase* parentObjectBase) :
	CollisionComponent(parentObjectBase)
{
}

MovingTriangleMeshCollisionComponent::~MovingTriangleMeshCollisionComponent()
{
}

void MovingTriangleMeshCollisionComponent::PreInit()
{
	GOKNAR_ASSERT(relativeMesh_ != nullptr);

	MeshUnit* subMesh = relativeMesh_->GetSubMeshes()[0];

	const VertexArray* vertexArray = subMesh->GetVerticesPointer();
	int vertexCount = subMesh->GetVertexCount();

	GOKNAR_ASSERT(0 < vertexCount);

	bulletCollisionShape_ = new btConvexHullShape(&vertexArray->at(0).position.x, vertexCount, sizeof(VertexData));

	CollisionComponent::PreInit();
}

void MovingTriangleMeshCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void MovingTriangleMeshCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
}
	
void MovingTriangleMeshCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void MovingTriangleMeshCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}