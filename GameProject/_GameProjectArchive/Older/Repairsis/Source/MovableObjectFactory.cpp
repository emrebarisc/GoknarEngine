#include "MovableObjectFactory.h"
#include "GameManager.h"
#include "GridManager.h"

MovableObjectFactory* MovableObjectFactory::instance_ = nullptr;
MovableObjectFactory* MovableObjectFactory::GetInstance()
{
	if (!instance_)
		instance_ =  new MovableObjectFactory();
	
	return instance_;
}

MovableObject* MovableObjectFactory::CreateMovableObject()
{
	if (GameManager::GetInstance()->GetIsGameOver())
	{
		return nullptr;
	}

	int type = rand() % 1;

	switch (type)
	{
	case 0: 
	{
		RectanglePart* newRectanglePart = new RectanglePart();
		GridManager::GetInstance()->SetCurrentObject(newRectanglePart);
		return newRectanglePart;
		break;
	}
	default:
		return nullptr;
	}
	
}

MovableObjectFactory::MovableObjectFactory()
{
	instance_ = this;
}


MovableObjectFactory::~MovableObjectFactory()
{
}
