#pragma once
#include "MovableObject.h"

class StaticMeshComponent;

class RectanglePart : public MovableObject
{
public:
	RectanglePart();
	~RectanglePart();

private:

	void SetParent(RepairableObject* parent);
	RepairableObject* GetParent();
	StaticMeshComponent* meshComponent_;
};

