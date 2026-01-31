#include "Characters/DefaultCharacter.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"

#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"

#include "Controllers/DefaultCharacterController.h"
#include "Components/DefaultCharacterMovementComponent.h"

DefaultCharacter::DefaultCharacter() :
	BaseCharacter()
{
	SetIsTickable(true);

	SetName("DefaultCharacter");
	SetTag("Character");

	capsuleCollisionComponent_->SetRadius(0.4f);
	capsuleCollisionComponent_->SetHeight(0.9f);
	capsuleCollisionComponent_->SetCollisionGroup(GetCollisionGroup());

	movementComponent_ = AddSubComponent<DefaultCharacterMovementComponent>();
	movementComponent_->SetParent(GetRootComponent());

	controller_ = new DefaultCharacterController(this);

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SK_DefaultCharacter.fbx");
	skeletalMesh_->GetMaterial()->SetSpecularReflectance( Vector3{1.f} );
	skeletalMesh_->GetMaterial()->SetBaseColor( Vector3{1.f} );
	skeletalMesh_->GetMaterial()->SetAmbientReflectance( Vector3{1.f} );

	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetParent(GetRootComponent());
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.01f });
	skeletalMeshComponent_->SetRelativePosition(Vector3{ 0.f, 0.f, -0.7f });

	thirdPersonCameraComponent_ = AddSubComponent<CameraComponent>();
	thirdPersonCameraComponent_->SetCameraFollowsComponentRotation(false);
	thirdPersonCameraComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 0.f, 0.f, 0.f }));
	thirdPersonCameraComponent_->SetRelativePosition(Vector3::UpVector * 2.f + thirdPersonCameraComponent_->GetRelativeForwardVector() * -8.f);
	thirdPersonCameraComponent_->SetParent(GetRootComponent());

	Camera* thirdPersonCamera = thirdPersonCameraComponent_->GetCamera();
	thirdPersonCamera->SetNearDistance(1.f);
	thirdPersonCamera->SetFarDistance(1000.f);
	thirdPersonCamera->SetFOV(45.f);
}

DefaultCharacter::~DefaultCharacter()
{
}

void DefaultCharacter::BeginGame()
{
	RaycastData raycastData;
	raycastData.from = Vector3{0.f, 0.f, 1000.f};
	raycastData.to = Vector3{0.f, 0.f, -1000.f};

	RaycastSingleResult raycastResult;

	engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

	SetWorldPosition(raycastResult.hitPosition);

	Idle();

	cameraDistance_.Reset(defaultCameraDistance_);
	cameraDistance_.speed = 32.f;
}

void DefaultCharacter::Tick(float deltaTime)
{
	BaseCharacter::Tick(deltaTime);

	const Vector2i cursorMovement = ((DefaultCharacterController*)controller_)->GetCursorDeltaMoveLastFrame();

	float multiplier = mouseSensitivity_ * deltaTime;

	cameraYaw_ -= cursorMovement.x * multiplier;
	cameraPitch_ += cursorMovement.y * multiplier;

	GoknarMath::Clamp(cameraPitch_, minPitch_, maxPitch_);

	Quaternion yawRot = Quaternion::FromAxisAngle(Vector3::UpVector, cameraYaw_);
	Quaternion pitchRot = Quaternion::FromAxisAngle(Vector3::LeftVector, cameraPitch_);

	Quaternion newRotation = yawRot * pitchRot;

	Vector3 baseOffset = Vector3{ -1.f, 0.f, 0.f };

	Vector3 cameraPosition = thirdPersonCameraComponent_->GetWorldPosition();
	Vector3 raycastFromPosition = GetWorldPosition() + Vector3{ 0.f, 0.f, 2.f };

	RaycastData raycastData;
	raycastData.from = raycastFromPosition;
	raycastData.to = cameraPosition;
	raycastData.collisionGroup = CollisionGroup::AllBlock;
	raycastData.collisionMask = CollisionMask::BlockAllExceptCharacter;

	RaycastSingleResult raycastResult;
	if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult))
	{
		cameraDistance_.UpdateDestination((raycastResult.hitPosition - raycastFromPosition).Length());
	}
	else
	{
		cameraDistance_.UpdateDestination(defaultCameraDistance_);
	}

	Vector3 newRelativePosition = Vector3::UpVector + (newRotation * baseOffset) * cameraDistance_.current;

	thirdPersonCameraComponent_->SetRelativeRotation(newRotation);
	thirdPersonCameraComponent_->SetRelativePosition(newRelativePosition);
}

void DefaultCharacter::Die()
{
	BaseCharacter::Die();
}
