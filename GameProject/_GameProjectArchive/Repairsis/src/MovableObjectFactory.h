#pragma once
#include "RectanglePart.h"
#include "MovableObject.h"

class Game;

class MovableObjectFactory
{
public:
	friend Game;

	static MovableObjectFactory* GetInstance();
	MovableObject* CreateMovableObject();

private:
	MovableObjectFactory();
	~MovableObjectFactory();
	static MovableObjectFactory* instance_;
};

