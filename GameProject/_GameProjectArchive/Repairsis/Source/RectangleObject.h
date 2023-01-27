#pragma once
#include "RectanglePart.h"
#include "RepairableObject.h"

class RectangleObject : public RepairableObject
{
public:
	RectangleObject();
	~RectangleObject();

	void AddPartToRectangle(MovableObject* part);
private:
	RectanglePart* topLeft;
	RectanglePart* topRight;
	RectanglePart* bottomLeft;
	RectanglePart* bottomRight;

	bool CheckRepaired();
};

