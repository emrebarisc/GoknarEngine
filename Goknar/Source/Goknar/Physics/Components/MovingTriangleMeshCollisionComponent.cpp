#include "pch.h"

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"

#include "ObjectBase.h"
#include "MovingTriangleMeshCollisionComponent.h"
#include "Model/MeshUnit.h"
#include "Physics/PhysicsUtils.h"

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

	const VertexArray* vertexArray = relativeMesh_->GetVerticesPointer();
	int vertexCount = relativeMesh_->GetVertexCount();

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