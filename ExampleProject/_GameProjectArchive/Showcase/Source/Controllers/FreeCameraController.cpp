#include "pch.h"

#include "FreeCameraController.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Debug/DebugDrawer.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/IMeshInstance.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/MultipleCollisionComponent.h"
#include "Goknar/Physics/Components/MovingTriangleMeshCollisionComponent.h"

#include "Objects/FreeCameraObject.h"

#include "Game.h"
#include "ArcherCharacter.h"
#include "ArcherCharacterController.h"

#include "Objects/PhysicsBox.h"
#include "Objects/PhysicsCapsule.h"
#include "Objects/PhysicsSphere.h"
#include "Objects/MultipleCollisionComponentObject.h"
#include "Objects/Monkey.h"

static bool drawDebugObjects_ = true;

FreeCameraController::FreeCameraController(FreeCameraObject* freeCameraObject) :
	freeCameraObject_(freeCameraObject),
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false)
{
	movementSpeed_ = 50.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);

	onMouseRightClickPressedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseRightClickPressed>(this);
	onMouseRightClickReleasedDelegate_ = KeyboardDelegate::Create < FreeCameraController, &FreeCameraController::OnMouseRightClickReleased >(this);
	onMouseMiddleClickPressedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseMiddleClickPressed>(this);
	onMouseMiddleClickReleasedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseMiddleClickReleased>(this);

	moveLeftDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveLeftListener>(this);
	moveRightDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveRightListener>(this);
	moveForwardDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveForwardListener>(this);
	moveBackwardDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveBackwardListener>(this);
	moveUpDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveUpListener>(this);
	moveDownDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);

	doRaycastingDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::Raycast>(this);
	doSweepingDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::Sweep>(this);

	switchToFreeCameraDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::SwitchToFreeCamera>(this);

	onScrollMoveDelegate_ = Delegate<void(double, double)>::Create<FreeCameraController, &FreeCameraController::ScrollListener>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::Create<FreeCameraController, &FreeCameraController::CursorMovement>(this);

	toggleTimeScaleDelegate_ = []() { engine->SetTimeScale(0.f < engine->GetTimeScale() ? 0.f : 1.f); };

	toggleDebugObjectsDelegate_ = []()
		{
			Game* game = dynamic_cast<Game*>(engine->GetApplication());
			if (game->GetDrawDebugObjects())
			{
				const std::vector<DebugObject*> debugObjects = engine->GetObjectsOfType<DebugObject>();
				for (auto debugObject : debugObjects)
				{
					debugObject->Destroy();
				}
			}
			else
			{
				const std::vector<PhysicsBox*> physicsBoxObjects = engine->GetObjectsOfType<PhysicsBox>();
				for (auto debugObject : physicsBoxObjects)
				{
					DebugDrawer::DrawCollisionComponent(dynamic_cast<BoxCollisionComponent*>(debugObject->GetCollisionComponent()), Colorf::Yellow, 5.f, 0.5f);
				}

				const std::vector<PhysicsSphere*> physicsSphereObjects = engine->GetObjectsOfType<PhysicsSphere>();
				for (auto debugObject : physicsSphereObjects)
				{
					DebugDrawer::DrawCollisionComponent(dynamic_cast<SphereCollisionComponent*>(debugObject->GetCollisionComponent()), Colorf::Red, 5.f, 0.5f);
				}

				const std::vector<PhysicsCapsule*> physicsCapsuleObjects = engine->GetObjectsOfType<PhysicsCapsule>();
				for (auto debugObject : physicsCapsuleObjects)
				{
					DebugDrawer::DrawCollisionComponent(dynamic_cast<CapsuleCollisionComponent*>(debugObject->GetCollisionComponent()), Colorf::Red, 5.f, 0.5f);
				}

				const std::vector<MultipleCollisionComponentObject*> multipleCollisionComponentObjects = engine->GetObjectsOfType<MultipleCollisionComponentObject>();
				for (auto debugObject : multipleCollisionComponentObjects)
				{
					DebugDrawer::DrawCollisionComponent(dynamic_cast<SphereCollisionComponent*>(debugObject->GetSphereCollisionComponent1()), Colorf::Red, 5.f, 0.5f);
					DebugDrawer::DrawCollisionComponent(dynamic_cast<SphereCollisionComponent*>(debugObject->GetSphereCollisionComponent2()), Colorf::Red, 5.f, 0.5f);
					DebugDrawer::DrawCollisionComponent(dynamic_cast<BoxCollisionComponent*>(debugObject->GetBoxCollisionComponent()), Colorf::Red, 5.f, 0.5f);
				}

				const std::vector<Monkey*> monkeys = engine->GetObjectsOfType<Monkey>();
				for (auto debugObject : monkeys)
				{
					DebugDrawer::DrawCollisionComponent(dynamic_cast<MovingTriangleMeshCollisionComponent*>(debugObject->GetCollisionComponent()), Colorf::Magenta, 5.f, 0.5f);
				}
			}

			game->SetDrawDebugObjects(!game->GetDrawDebugObjects());
		};
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
		engine->GetCameraManager()->SetActiveCamera(freeCameraObject_->GetCameraComponent()->GetCamera());
		engine->GetInputManager()->SetIsCursorVisible(false);
	}
	else
	{
		UnbindInputDelegates();
	}
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
		Pitch(-cursorMovementVector.y);

		previousCursorPositionForRotating_ = currentCursorPosition;
	}

	if (isMovingCameraIn2D_)
	{
		Vector2 cursorMovementVector = currentCursorPosition - previousCursorPositionFor2DMovement_;

		MoveLeft(-cursorMovementVector.x);
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
	Vector3 newForwardVector = freeCameraObject_->GetWorldRotation().ToEulerRadians();
	freeCameraObject_->SetWorldRotation(Quaternion::FromEulerRadians(newForwardVector + Vector3{ 0.f, 0.f, value }));
}

void FreeCameraController::Pitch(float value)
{
	Vector3 newForwardVector = freeCameraObject_->GetForwardVector().RotateVectorAroundAxis(freeCameraObject_->GetLeftVector(), value);
	freeCameraObject_->SetWorldRotation(newForwardVector.GetRotationNormalized());
}

void FreeCameraController::OnMouseRightClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetMainWindow(), x, y);
	previousCursorPositionForRotating_ = Vector2(x, y);
	isRotatingTheCamera_ = true;
}

void FreeCameraController::OnMouseMiddleClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetMainWindow(), x, y);
	previousCursorPositionFor2DMovement_ = Vector2(x, y);
	isMovingCameraIn2D_ = true;
}

void FreeCameraController::MoveForward(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetCameraComponent()->GetWorldForwardVector() * 0.025f * multiplier);
}

void FreeCameraController::MoveLeft(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetCameraComponent()->GetWorldLeftVector() * 0.025f * multiplier);
}

void FreeCameraController::MoveUp(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetCameraComponent()->GetWorldUpVector() * 0.025f * multiplier);
}

void FreeCameraController::BindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, onMouseRightClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, onMouseRightClickReleasedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, onMouseMiddleClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, onMouseMiddleClickReleasedDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, moveLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_REPEAT, moveRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_REPEAT, moveForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_REPEAT, moveBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, moveUpDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, moveDownDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::F7, INPUT_ACTION::G_PRESS, doRaycastingDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F8, INPUT_ACTION::G_PRESS, doSweepingDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_PRESS, switchToFreeCameraDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::F3, INPUT_ACTION::G_PRESS, toggleTimeScaleDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, toggleDebugObjectsDelegate_);

	inputManager->AddCursorDelegate(onCursorMoveDelegate_);
	inputManager->AddScrollDelegate(onScrollMoveDelegate_);
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

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F7, INPUT_ACTION::G_PRESS, doRaycastingDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F8, INPUT_ACTION::G_PRESS, doSweepingDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_PRESS, switchToFreeCameraDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F3, INPUT_ACTION::G_PRESS, toggleTimeScaleDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, toggleDebugObjectsDelegate_);

	inputManager->RemoveCursorDelegate(onScrollMoveDelegate_);
	inputManager->RemoveScrollDelegate(onCursorMoveDelegate_);
}

void FreeCameraController::Raycast() const
{
	double cursorPositionX, cursorPositionY;
	engine->GetInputManager()->GetCursorPosition(cursorPositionX, cursorPositionY);
	Vector2i cursorPosition{ (int)cursorPositionX, (int)cursorPositionY };

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	Vector3 viewDirection = activeCamera->GetWorldDirectionAtPixel(cursorPosition);

	RaycastData raycastData;
	raycastData.from = activeCamera->GetPosition();
	raycastData.to = raycastData.from + viewDirection * 1000.f;

	RaycastSingleResult raycastResult;
	engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

	if (RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(raycastResult.hitObject))
	{
		hitRigidBody->ApplyForce(raycastResult.hitNormal * 1000.f);
	}
}

void FreeCameraController::Sweep() const
{
}

void FreeCameraController::SwitchToFreeCamera()
{
	UnbindInputDelegates();
	Game* game = dynamic_cast<Game*>(engine->GetApplication());

	engine->GetCameraManager()->SetActiveCamera(game->GetPhysicsArcher()->GetThirdPersonCameraComponent()->GetCamera());
	game->GetPhysicsArcher()->GetController()->SetIsActive(true);
}
