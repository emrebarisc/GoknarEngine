#include "pch.h"

#include "FreeCameraController.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Debug/DebugDrawer.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/IMeshInstance.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"

FreeCameraController::FreeCameraController() :
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false),
	freeCamera_(new Camera(Vector3::ZeroVector, Vector3::ForwardVector, Vector3::UpVector))
{
	movementSpeed_ = 50.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);

	onMouseRightClickPressedDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::OnMouseRightClickPressed>(this);
	onMouseRightClickReleasedDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveRightListener>(this);
	onMouseMiddleClickPressedDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::OnMouseRightClickReleased>(this);
	onMouseMiddleClickReleasedDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveRightListener>(this);

	moveLeftDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveLeftListener>(this);
	moveRightDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveRightListener>(this);
	moveForwardDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveForwardListener>(this);
	moveBackwardDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveBackwardListener>(this);
	moveUpDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveUpListener>(this);
	moveDownDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);

	doRaycastClosestTestDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);
	doRaycastAllTestDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);
	doSweepTestDelegate_ = KeyboardDelegate::create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);

	onScrollMoveDelegate_ = Delegate<void(double, double)>::create<FreeCameraController, &FreeCameraController::ScrollListener>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::create<FreeCameraController, &FreeCameraController::CursorMovement>(this);
}

FreeCameraController::~FreeCameraController()
{
	UnbindInputDelegates();
}

void FreeCameraController::BeginGame()
{
	if (GetIsActive())
	{
		BindInputDelegates();
	}

	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	freeCamera_->SetImageWidth(windowSize.x);
	freeCamera_->SetImageHeight(windowSize.y);
}

void FreeCameraController::SetupInputs()
{
}

void FreeCameraController::SetIsActive(bool isActive)
{
	Controller::SetIsActive(isActive);

	if (isActive)
	{
		BindInputDelegates();
	}
	else
	{
		UnbindInputDelegates();
	}
}

void FreeCameraController::CursorMovement(double x, double y)
{
	Vector2 currentCursorPosition(x, y);

	if (isRotatingTheCamera_)
	{
		Vector2 cursorMovementVector = (previousCursorPositionForRotating_ - currentCursorPosition) / 250.f;
		Yaw(cursorMovementVector.x);
		Pitch(cursorMovementVector.y);

		previousCursorPositionForRotating_ = currentCursorPosition;
	}

	if (isMovingCameraIn2D_)
	{
		Vector2 cursorMovementVector = currentCursorPosition - previousCursorPositionFor2DMovement_;

		MoveRight(cursorMovementVector.x);
		MoveUp(-cursorMovementVector.y);

		previousCursorPositionFor2DMovement_ = currentCursorPosition;
	}
}

void FreeCameraController::ScrollListener(double x, double y)
{
	MoveForward(y * movementSpeed_);
}

void FreeCameraController::Yaw(float value)
{
	freeCamera_->RotateAbout(Vector3::UpVector, value);
}

void FreeCameraController::Pitch(float value)
{
	freeCamera_->Pitch(value);
}

void FreeCameraController::DoRaycastClosestTest()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	Vector2i screenCoordinate = Vector2i{(int)x, (int)y};
	
	RaycastData raycastData;
	RaycastSingleResult raycastSingleResult;

	Vector3 cameraPosition = freeCamera_->GetPosition();
	raycastData.from = cameraPosition;
	raycastData.to = cameraPosition + 1000.f * freeCamera_->GetWorldDirectionAtPixel(screenCoordinate);
	if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastSingleResult))
	{
		PhysicsObject* hitObject = raycastSingleResult.hitObject;
		RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(hitObject);
		if (hitRigidBody)
		{
			hitRigidBody->ApplyForce(Vector3::UpVector * 10000.f);
			hitRigidBody->ApplyTorque(Vector3::LeftVector * 10000.f);
		}

		GOKNAR_INFO("Raycast hit object: {}", hitObject->GetName());
	}
}

void FreeCameraController::DoRaycastAllTest()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	Vector2i screenCoordinate = Vector2i{(int)x, (int)y};
	
	RaycastAllResult raycastAllResult;
	RaycastData raycastData;

	Vector3 cameraPosition = freeCamera_->GetPosition();
	raycastData.from = cameraPosition;
	raycastData.to = cameraPosition + 1000.f * freeCamera_->GetWorldDirectionAtPixel(screenCoordinate);
	if (engine->GetPhysicsWorld()->RaycastAll(raycastData, raycastAllResult))
	{
		int hitCount = raycastAllResult.hitResults.size();
		for(int hitIndex = 0; hitIndex < hitCount; ++hitIndex)
		{
			RaycastSingleResult raycastSingleResult = raycastAllResult.hitResults[hitIndex];
			PhysicsObject* hitObject = raycastSingleResult.hitObject;
			RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(hitObject);
			if (hitRigidBody)
			{
				hitRigidBody->ApplyForce(Vector3::UpVector * 10000.f);
				hitRigidBody->ApplyTorque(Vector3::LeftVector * 10000.f);
			}

			GOKNAR_INFO("\tRaycast hit object: {}", hitObject->GetName());
		}
	}
}

void FreeCameraController::DoSweepTest()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	Vector2i screenCoordinate = Vector2i{(int)x, (int)y};
	
	RaycastSingleResult raycastSingleResult;

	SweepData sweepData;

	BoxCollisionComponent* sweepCollisionComponent = new BoxCollisionComponent(static_cast<Component*>(nullptr));

	sweepCollisionComponent->SetHalfSize(Vector3{1.f, 1.f, 1.f});
	sweepCollisionComponent->PreInit();
	sweepCollisionComponent->Init();
	sweepCollisionComponent->PostInit();

	sweepData.collisionComponent = sweepCollisionComponent;

	Vector3 cameraPosition = freeCamera_->GetPosition();
	Quaternion cameraRotation = Quaternion(freeCamera_->GetViewMatrix());

	sweepData.fromPosition = cameraPosition;
	sweepData.fromRotation = cameraRotation;
	sweepData.toPosition = cameraPosition + 1000.f * freeCamera_->GetWorldDirectionAtPixel(screenCoordinate);
	sweepData.toRotation = cameraRotation;

	sweepCollisionComponent->SetRelativePosition(cameraPosition);
	sweepCollisionComponent->SetRelativeRotation(cameraRotation);
	// DebugDrawer::DrawCollisionComponent(sweepCollisionComponent, Colorf::Blue, 10.f, 0.f);

	if (engine->GetPhysicsWorld()->SweepClosest(sweepData, raycastSingleResult))
	{
		PhysicsObject* hitObject = raycastSingleResult.hitObject;
		RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(hitObject);
		if (hitRigidBody)
		{
			hitRigidBody->ApplyForce(Vector3::UpVector * 10000.f);
			hitRigidBody->ApplyTorque(Vector3::LeftVector * 10000.f);
		}

		GOKNAR_INFO("Raycast hit object: {}", hitObject->GetName());
	}

	sweepCollisionComponent->Destroy();
}

void FreeCameraController::OnMouseRightClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionForRotating_ = Vector2(x, y);
	isRotatingTheCamera_ = true;
}

void FreeCameraController::OnMouseMiddleClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionFor2DMovement_ = Vector2(x, y);
	isMovingCameraIn2D_ = true;
}

void FreeCameraController::MoveForward(float multiplier/* = 1.f*/)
{
	freeCamera_->MoveForward(engine->GetDeltaTime() * multiplier);
}

void FreeCameraController::MoveRight(float multiplier/* = 1.f*/)
{
	freeCamera_->MoveRight(engine->GetDeltaTime() * multiplier);
}

void FreeCameraController::MoveUp(float multiplier/* = 1.f*/)
{
	freeCamera_->MoveUpward(engine->GetDeltaTime() * multiplier);
}

void FreeCameraController::BindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, onMouseRightClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, onMouseRightClickReleasedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, onMouseMiddleClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, onMouseMiddleClickReleasedDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT, INPUT_ACTION::G_REPEAT, moveLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::RIGHT, INPUT_ACTION::G_REPEAT, moveRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::UP, INPUT_ACTION::G_REPEAT, moveForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::DOWN, INPUT_ACTION::G_REPEAT, moveBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, moveUpDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, moveDownDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::NUM_8, INPUT_ACTION::G_PRESS, doRaycastClosestTestDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::NUM_9, INPUT_ACTION::G_PRESS, doRaycastAllTestDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::NUM_0, INPUT_ACTION::G_PRESS, doSweepTestDelegate_);

	inputManager->AddCursorDelegate(onScrollMoveDelegate_);
	inputManager->AddScrollDelegate(onCursorMoveDelegate_);

}

void FreeCameraController::UnbindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, onMouseRightClickPressedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, onMouseRightClickReleasedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, onMouseMiddleClickPressedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, onMouseMiddleClickReleasedDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::LEFT, INPUT_ACTION::G_REPEAT, moveLeftDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::RIGHT, INPUT_ACTION::G_REPEAT, moveRightDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::UP, INPUT_ACTION::G_REPEAT, moveForwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::DOWN, INPUT_ACTION::G_REPEAT, moveBackwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, moveUpDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, moveDownDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::NUM_8, INPUT_ACTION::G_PRESS, doRaycastClosestTestDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::NUM_9, INPUT_ACTION::G_PRESS, doRaycastAllTestDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::NUM_0, INPUT_ACTION::G_PRESS, doSweepTestDelegate_);

	inputManager->RemoveCursorDelegate(onScrollMoveDelegate_);
	inputManager->RemoveScrollDelegate(onCursorMoveDelegate_);
}
