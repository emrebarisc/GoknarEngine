#include "RectangleObject.h"
#include "GridManager.h"
#include "GameManager.h"

RectangleObject::RectangleObject()
{
	topLeft = topRight = bottomLeft = bottomRight = nullptr;
}


RectangleObject::~RectangleObject()
{
}

bool RectangleObject::CheckRepaired()
{
	return topLeft != nullptr && topRight != nullptr && bottomLeft != nullptr && bottomRight != nullptr;
}

void RectangleObject::AddPartToRectangle(MovableObject* part)
{
	switch (part->GetType()) {
	case 0:
		if (bottomLeft == nullptr) {
			bottomLeft = dynamic_cast<RectanglePart*>(part);
		}
		break;
	case 1:
		if (bottomRight == nullptr) {
			bottomRight = dynamic_cast<RectanglePart*>(part);
		}
		break;
	case 2:
		if (topLeft == nullptr) {
			topLeft = dynamic_cast<RectanglePart*>(part);
		}
		break;
	case 3:
		if (topRight == nullptr) {
			topRight = dynamic_cast<RectanglePart*>(part);
		}
		break;
	}
	if (CheckRepaired()) {
		GameManager::GetInstance()->IncreaseScore();

		GridManager::GetInstance()->ClearRepairedObject(bottomLeft);
		GridManager::GetInstance()->ClearRepairedObject(bottomRight);
		GridManager::GetInstance()->ClearRepairedObject(topLeft);
		GridManager::GetInstance()->ClearRepairedObject(topRight);
	}
}
