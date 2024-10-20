#include "FreeCameraObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Camera.h"
#include "Goknar/Debug/DebugDrawer.h"
#include "Goknar/Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Controllers/FreeCameraController.h"

#include "Arrow.h"
#include "CannonBall.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	SetIsTickable(true);
	SetIsTickEnabled(true);

	cameraComponent_ = AddSubComponent<CameraComponent>();
	cameraComponent_->SetCameraFollowsComponentRotation(true);
	SetRootComponent(cameraComponent_);

	freeCameraController_ = new FreeCameraController(this);
	freeCameraController_->SetName("FreeCameraController");
}

void FreeCameraObject::BeginGame()
{
	WindowManager* windowManager = engine->GetWindowManager();
	Camera* camera = cameraComponent_->GetCamera();
	camera->SetImageWidth(windowManager->GetWindowSize().x);
	camera->SetImageHeight(windowManager->GetWindowSize().y);
}

#define GET_DIRECTION(followObject) followObject->GetForwardVector() - Vector3::UpVector
#define GET_POSITION(followObject)  followObject->GetWorldPosition() + (dynamic_cast<Arrow*>(followObject) ? 2.f * followObject->GetForwardVector() : Vector3::ZeroVector) - (dynamic_cast<Arrow*>(followObject) ? 4.f : 8.f) * GetForwardVector()

void FreeCameraObject::Tick(float deltaTime)
{
	if (followObject_)
	{
		Vector3 springPosition = GetSpringPosition(deltaTime);
		SetWorldPosition(springPosition);
	}
}

void FreeCameraObject::SetFollowObject(ObjectBase* followObject)
{
	followObject_ = followObject;

	if (followObject_)
	{
		Arrow* arrow = dynamic_cast<Arrow*>(followObject);
		RigidBody* rigidbody = dynamic_cast<RigidBody*>(followObject);
		if (!arrow && (!rigidbody || rigidbody->GetMass() <= 0.f))
		{
			SetIsTickEnabled(false);
		}
		else
		{
			SetIsTickEnabled(true);
		}

		if (arrow)
		{
			Vector3 direction = GET_DIRECTION(followObject_);
			SetWorldRotation(direction.GetRotationNormalized(), false);

			Vector3 position = GET_POSITION(followObject_);
			SetWorldPosition(position);
		}
	}
}

Vector3 FreeCameraObject::GetSpringPosition(float deltaTime)
{
	const float springConstant = 10.f;
	const float damping = 0.1f;
	const float desiredDistance = 2.f;

	Vector3 direction = GET_DIRECTION(followObject_);
	Vector3 desiredPosition = GET_POSITION(followObject_);
	Vector3 springPosition = GetWorldPosition();

	Vector3 positionDifference = desiredPosition - springPosition;
	Vector3 springForce = positionDifference * springConstant;

	Vector3 velocity = springForce - (currentVelocity_ * damping);

	springPosition += velocity * deltaTime;

	return springPosition;
}