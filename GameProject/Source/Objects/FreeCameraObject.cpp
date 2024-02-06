#include "FreeCameraObject.h"

#include "Controllers/FreeCameraController.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	freeCameraController_ = AddSubComponent<FreeCameraController>();
}

void FreeCameraObject::BeginGame()
{
}

void FreeCameraObject::Tick(float deltaTime)
{

}
