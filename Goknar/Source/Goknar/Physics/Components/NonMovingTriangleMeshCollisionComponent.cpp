#include "pch.h"

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"

#include "ObjectBase.h"
#include "NonMovingTriangleMeshCollisionComponent.h"
#include "Model/MeshUnit.h"
#include "Physics/PhysicsUtils.h"

#include "Log.h"

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

	//2----1
	//|   /|
	//|  / |
	//| /  |
	//|/   |
	//3----0

	//btVector3 quad[] = {
	//	btVector3(100, -100, 1),
	//	btVector3(100, 100, 1),
	//	btVector3(-100, 100, 1),
	//	btVector3(-100, -100, 1) };

	//bulletTriangleMesh_->addTriangle(quad[0], quad[1], quad[2], true);
	//bulletTriangleMesh_->addTriangle(quad[0], quad[2], quad[3], true);

	//btVector3 quad[] = {
	//	btVector3(0, 100, -100),
	//	btVector3(0, 100, 100),
	//	btVector3(0, -100, 100),
	//	btVector3(0, -100, -100) };

	//btTriangleMesh* mesh = new btTriangleMesh();
	//mesh->addTriangle(quad[0], quad[1], quad[2], true);
	//mesh->addTriangle(quad[0], quad[2], quad[3], true);

	//btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(mesh, true, true);
	//bulletCollisionShape_ = trimesh;

	CollisionComponent::PreInit();
}

void NonMovingTriangleMeshCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void NonMovingTriangleMeshCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();
	
	if(!isScalable_)
	{
		delete bulletTriangleMesh_;
		bulletTriangleMesh_ = nullptr;
	}
}
	
void NonMovingTriangleMeshCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void NonMovingTriangleMeshCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}

void NonMovingTriangleMeshCollisionComponent::UpdateTransformation()
{
	if(!isScalable_)
	{
		return;
	}

	CollisionComponent::UpdateTransformation();
}