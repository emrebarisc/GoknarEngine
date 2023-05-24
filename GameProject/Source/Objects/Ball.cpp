#include "Ball.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"

Ball::Ball() : RigidBody()
{
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Ball.fbx");
	staticMeshComponent_->SetMesh(staticMesh);
}