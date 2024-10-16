#include "Arrow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Goknar/Debug/DebugDrawer.h"

#include "Components/ProjectileMovementComponent.h"
#include "Objects/AxisObject.h"

#include "Physics/Components/BoxCollisionComponent.h"

#include "Game.h"
#include "Objects/FreeCameraObject.h"
#include "ArcherCharacter.h"
#include "ArcherCharacterController.h"

#include "Goknar/Camera.h"
#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Managers/CameraManager.h"

Arrow::Arrow() : OverlappingPhysicsObject()
{
	StaticMesh* arrowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Arrow.fbx");

	overlappingCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();

	Vector3 arrowMeshSize = arrowStaticMesh->GetAABB().GetSize();

	overlappingCollisionComponent_->SetRelativePosition(Vector3(arrowMeshSize.x * 0.5f, 0.f, 0.f));
	overlappingCollisionComponent_->SetHalfSize(arrowMeshSize * Vector3(0.5f, 1.f, 0.25f));
	overlappingCollisionComponent_->OnOverlapBegin = Delegate<OverlapBeginAlias>::Create<Arrow, &Arrow::OnOverlapBegin>(this);
	overlappingCollisionComponent_->OnOverlapContinue = Delegate<OverlapContinueAlias>::Create<Arrow, &Arrow::OnOverlapContinue>(this);
	overlappingCollisionComponent_->OnOverlapEnd = Delegate<OverlapEndAlias>::Create<Arrow, &Arrow::OnOverlapEnd>(this);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent_->SetRelativePosition(Vector3(-arrowMeshSize.x * 0.5f, 0.f, 0.f));
	staticMeshComponent_->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
	movementComponent_->SetIsActive(false);

	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	engine->GetCameraManager()->SetActiveCamera(game->GetPhysicsArcher()->GetController()->GetThirdPersonCameraComponent()->GetCamera());
	game->GetFreeCameraObject()->SetFollowObject(nullptr);
}

void Arrow::BeginGame()
{
	ObjectBase::BeginGame();

	//DebugDrawer::DrawCollisionComponent(overlappingCollisionComponent_, Colorf::Orange, 1.f);
}

void Arrow::Shoot()
{
	movementComponent_->Shoot();

	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	engine->GetCameraManager()->SetActiveCamera(game->GetFreeCameraObject()->GetCameraComponent()->GetCamera());
	game->GetFreeCameraObject()->SetFollowObject(this);
	
}

void Arrow::OnOverlapBegin(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	if(otherObject->GetTag() == "Archer")
	{
		return;
	}

	SetParent(otherObject);
	overlappingCollisionComponent_->SetIsActive(false);
	movementComponent_->SetIsActive(false);

	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	game->GetFreeCameraObject()->SetFollowObject(nullptr);
	game->GetFreeCameraObject()->SetWorldRotation((-hitNormal).GetRotationNormalized());
	game->GetFreeCameraObject()->SetWorldPosition(hitPosition);
	game->GetFreeCameraObject()->SetParent(this);
}

void Arrow::OnOverlapContinue(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
}

void Arrow::OnOverlapEnd(PhysicsObject* otherObject, CollisionComponent* otherComponent)
{
}