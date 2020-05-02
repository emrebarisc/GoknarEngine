#pragma once

class MovableObject;

class RepairableObject
{
public:
	RepairableObject();
	~RepairableObject();

	virtual void AddPartToRectangle(MovableObject* part) = 0;
	virtual bool CheckRepaired() = 0;
};

