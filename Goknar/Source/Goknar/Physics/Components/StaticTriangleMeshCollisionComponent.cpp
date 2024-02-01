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
	delete bulletTriangleMesh_;
}

void StaticTriangleMeshCollisionComponent::PreInit()
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
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[1]).position),
			PhysicsUtils::FromVector3ToBtVector3(vertexArray->at(face.vertexIndices[2]).position)
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

void StaticTriangleMeshCollisionComponent::Init()
{
	CollisionComponent::Init();
}

void StaticTriangleMeshCollisionComponent::PostInit()
{
	CollisionComponent::PostInit();

	// If bulletTriangleMesh_ is deleted here, 
	// it cannot be resized via 
	// void ObjectBase::SetRelativeScaling(const Vector3&)
	delete bulletTriangleMesh_;
	bulletTriangleMesh_ = nullptr;
}
	
void StaticTriangleMeshCollisionComponent::BeginGame()
{
	CollisionComponent::BeginGame();
}

void StaticTriangleMeshCollisionComponent::TickComponent(float deltaTime)
{
	CollisionComponent::TickComponent(deltaTime);

	
}