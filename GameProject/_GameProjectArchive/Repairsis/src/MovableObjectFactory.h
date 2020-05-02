#pragma once
#include "RectanglePart.h"
#include "MovableObject.h"

class MovableObjectFactory
{
public:
	static MovableObjectFactory* GetInstance();
	MovableObject* CreateMovableObject();

private:
	MovableObjectFactory();
	~MovableObjectFactory();
	static MovableObjectFactory* instance_;
};

