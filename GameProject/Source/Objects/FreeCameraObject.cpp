#include "FreeCameraObject.h"

#include "Controllers/FreeCameraController.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	freeCameraController_ = new FreeCameraController();
}

void FreeCameraObject::BeginGame()
{
}

void FreeCameraObject::Tick(float deltaTime)
{

}
