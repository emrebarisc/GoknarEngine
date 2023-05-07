#include "RigidBodyObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"

RigidBodyObject::RigidBodyObject() : RigidBody()
{
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Box.fbx");
	staticMeshComponent_->SetMesh(staticMesh);
}