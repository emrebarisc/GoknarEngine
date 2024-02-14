#include "pch.h"

#include "FreeCameraController.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/IMeshInstance.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/RigidBody.h"

FreeCameraController::FreeCameraController(Component* parent) :
	Component(parent),
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false),
	freeCamera_(new Camera(Vector3::ZeroVector, Vector3::ForwardVector, Vector3::UpVector))
{
	movementSpeed_ = 50.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);
}

FreeCameraController::~FreeCameraController()
{
}

void FreeCameraController::BeginGame()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_LEFT, INPUT_ACTION::G_PRESS, std::bind(&FreeCameraController::OnMouseLeftClickPressed, this));


	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, std::bind(&FreeCameraController::OnMouseRightClickPressed, this));
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, std::bind(&FreeCameraController::OnMouseRightClickReleased, this));

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, std::bind(&FreeCameraController::OnMouseMiddleClickPressed, this));
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, std::bind(&FreeCameraController::OnMouseMiddleClickReleased, this));

	inputManager->AddScrollDelegate(std::bind(&FreeCameraController::ScrollListener, this, std::placeholders::_1, std::placeholders::_2));

	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveLeftListener, this));
	inputManager->AddKeyboardInputDelegate(KEY_MAP::RIGHT, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveRightListener, this));
	inputManager->AddKeyboardInputDelegate(KEY_MAP::UP, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveForwardListener, this));
	inputManager->AddKeyboardInputDelegate(KEY_MAP::DOWN, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveBackwardListener, this));
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveDownListener, this));
	inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, std::bind(&FreeCameraController::MoveUpListener, this));

	inputManager->AddCursorDelegate(std::bind(&FreeCameraController::CursorMovement, this, std::placeholders::_1, std::placeholders::_2));

	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	freeCamera_->SetImageWidth(windowSize.x);
	freeCamera_->SetImageHeight(windowSize.y);
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

void FreeCameraController::OnMouseLeftClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	Vector2i screenCoordinate = Vector2i{(int)x, (int)y};
	
	RaycastData raycastData;
	RaycastClosestResult raycastClosestResult;

	Vector3 cameraPosition = freeCamera_->GetPosition();
	raycastData.from = cameraPosition;
	raycastData.to = cameraPosition + 1000.f * freeCamera_->GetWorldDirectionAtPixel(screenCoordinate);
	if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastClosestResult))
	{
		PhysicsObject* hitObject = raycastClosestResult.hitObject;
		RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(hitObject);
		if (hitRigidBody)
		{
			//hitRigidBody->SetIsActive(false);
			hitRigidBody->ApplyForce(Vector3::UpVector * 100000.f);
		}

		GOKNAR_INFO("Raycast hit object: {}", hitObject->GetName());
	}
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